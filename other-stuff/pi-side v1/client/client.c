#include "rpi.h"
#include "sw-uart-veneer.c"
#include "pi-sd.h"
#include "fat32.h"
#include "libc/fast-hash32.h"
#include "circular-queue.c"

// this is the "main" function for the client.
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

    while (1) {
        // TODO if there is a message from the server, read it and break
    }

    // TODO:
    // pi_file_t *f = READ_FROM_ESP()
    void *ptr = (void *)0x80000;
    memcpy(ptr, f->data, f->n_data);
    BRANCHTO(0x80010);

    printk("Job complete!\n");
    clean_reboot();

    // TODO: 
    // SEND_ESP_MESSAGE TO SERVER (AND TRIGGER INTERRUPT)

    return 0;
}