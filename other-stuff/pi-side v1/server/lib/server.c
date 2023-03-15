#include "rpi.h"
#include "pi-sd.h"
#include "fat32.h"
#include "libc/fast-hash32.h"
#include "circular-queue.c"
#include "vector-base.h"

// client status bitfield
static volatile unsigned clients = 0;

enum client_status {
    BUSY,
    AVAILABLE
};

// a bitfield where each bit represents whether a client is available
// 0 = busy, 1 = available
unsigned get_client_status(unsigned client_id) {
    return (clients >> client_id) & AVAILABLE;
}
void set_client_status(unsigned client_id, unsigned client_status) {
    clients &= ~(1 << client_id);
    clients |= (client_status << client_id);
}

// clients will interrupt the server with a message when available
void interrupt_vector(unsigned pc) {
    dev_barrier();

    // TODO parse client ID from message header
    // TODO update client status with set_client_status

    dev_barrier();
    return;
}

static int interrupt_fn_t(unsigned client_id) {
    set_client_status(client_id, AVAILABLE);
    return 1;
}

pi_file_t* read_function(char* name, fat32_fs_t *fs, pi_dirent_t *root) {
    printk("Looking for %s.\n", name);
    pi_dirent_t *d = fat32_stat(fs, root, name);
    demand(d, "%s not found!\n", name);

    printk("Reading %s\n", name);
    pi_file_t *f = fat32_read(fs, root, name);
    assert(f);

    printk("crc of %s (nbytes=%d) = %x\n", name, f->n_data, 
            fast_hash(f->data,f->n_data));

    uint32_t *p = (void*)f->data;
    for(int i = 0; i < 4; i++) 
        printk("p[%d]=%x (%d)\n", i,p[i],p[i]);

    // verify checksum cookie at offset 0.
    assert(p[0] == 0x12345678);

    // verify was linked at 0x80000
    uint32_t addr = p[2];
    assert(addr == 0x80000);

    return f;
}

// this is the "main" function for the server.
void notmain(void) {
    // drivers
    uart_init();    
    kmalloc_init();
    pi_sd_init();
    // queue
    CircularQueue queue;
    initializeQueue(&queue, sizeof(pi_file_t), 10);
    // interrupt vector
    extern uint32_t interrupt_vec[];
    vector_base_set((void *)interrupt_vec);

    printk("Reading the MBR.\n");
    mbr_t *mbr = mbr_read();
    printk("Loading the first partition.\n");
    mbr_partition_ent_t partition;
    memcpy(&partition, mbr->part_tab1, sizeof(mbr_partition_ent_t));
    assert(mbr_part_is_fat32(partition.part_type));
    printk("Loading the FAT.\n");
    fat32_fs_t fs = fat32_mk(&partition);
    printk("Loading the root directory.\n");
    pi_dirent_t root = fat32_get_root(&fs);

    // manually load jobs from SD card
    pi_file_t *jobOne = read_function("job1.bin", &fs, &root);
    pi_file_t *jobTwo = read_function("job2.bin", &fs, &root);
    pi_file_t *jobThree = read_function("job3.bin", &fs, &root);
    enqueue(&queue, &jobOne);
    enqueue(&queue, &jobTwo);
    enqueue(&queue, &jobThree);

    pi_file_t dequeuedJob;
    while (dequeue(&queue, &dequeuedJob)) {
        printk("Dequeued job. n_alloc: %d, n_data: %d\n", dequeuedJob.n_alloc, dequeuedJob.n_data);
        
        // wait for an available client
        for (int i = 0; i < sizeof(clients); i++) {
            // if the client is available, send the job
            if (get_client_status(i)) {
                set_client_status(i, BUSY);
                // TODO send job to client over ESP
                break;
            }
            // if we've reached the end of the list, start over
            if (i == sizeof(clients) - 1) {
                i = 0;
            }
        }
    }

    return;
}