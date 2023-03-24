#include "rpi.h"
#include "libc/fast-hash32.h"
#include "vector-base.h"
#include "sw-uart.h"
#include "get-code.h"
#include "rpi-interrupts.h"
#include "constant.h"
#include "constants.h"
#include "pi-esp.h"

// sw_uart_t u;

static unsigned
has_data_timeout(unsigned timeout) {
    uint32_t start = timer_get_usec();
    while (1) {
        if (uart_has_data())
            return 1;
        uint32_t end = timer_get_usec();
        if ((end - start) >= timeout)
            return 0;
    }
}

static void wait_for_op(unsigned usec_timeout) {
    do {
        boot_put32(PI_READY);
    } while (has_data_timeout(usec_timeout) == 0);
}


void sw_uart_putk_ignorenull(sw_uart_t *uart, const char *msg, size_t len) {
    size_t i;
    for (i = 0; i < len; i++) {
        sw_uart_put8(uart, msg[i]);
    }
}

void synchronize() {
    wait_for_op(300 * 1000);
    unsigned op = boot_get32();
    if (op != UNIX_READY) {
        boot_err(BOOT_ERROR, "Expected UNIX_READY");
        rpi_reboot();
    }
    boot_put32(ACK);
}

void run_prog(void) {
    uint32_t nbytes = boot_get32();
    uint32_t crc = boot_get32();
    uint32_t n = boot_get32();
    uint8_t pis[n];
    for (int i = 0; i < n; i++) {
        pis[i] = uart_get8();
    }

    boot_put32(n);
    boot_put32(crc);

    uint32_t op = boot_get32();
    if (op != SEND_CODE) {
        boot_err(BOOT_ERROR, "Expected PUT_CODE");
        rpi_reboot();
    }

    uint32_t addr = 0x80000;
    uint8_t buf[8000];
    // memcpy(buf, "START:", 6);

    for (int i = 0; i < nbytes; i++) {
        uint8_t byte = uart_get8();
        PUT8(addr + i, byte);
        buf[i] = byte;
    }
    // memcpy(buf + 6 + nbytes, ":END", 4);

    // uint32_t *header = (void *)addr;
    uint32_t *header = (uint32_t *) buf;
    // Confirm info in headers
    assert(header[0] == 0x12345678);
    assert(header[2] == 0x80000);

    boot_put32(CODE_GOT);

    // TODO: Test this
    // for (int i = 0; i < n; i++) {
        // sendProgram(pis[i], buf, nbytes);
    // }

    sendProgram(2, buf, nbytes);


    // sw_uart_putk_ignorenull(&u, buf, 8000);
    // }
    // BRANCHTO(0x80010);

    boot_put32(DONE);

    (void)pis[0];
}

void init(){
    // repetitive, sw uart init does it , but here for sanity 
    gpio_set_input(21);
    gpio_set_pullup(21);
    gpio_set_output(23);
    

    u = (sw_uart_t*) kmalloc(sizeof(sw_uart_t));
    *u = sw_uart_init(23,21,9600);
    char* buff = kmalloc(sizeof(char) * 32);
    
    init_fileTable();
    int_init();
    gpio_int_falling_edge(21);
    system_enable_interrupts();
    
   // printk("init server:\n");
  //  send_cmd(u,ESP_SEND_DATA,0xf,0xf,"DOES THIS WORK",25);
    printk("about to inti server\n");
    int servIP = server_init();
  //  while(servIP == -1) servIP = server_init();
    printk("server ip = %x\n",servIP);
}

void notmain(void) {
    // uart_init();
    // u = sw_uart_init(4, 5, 9600);

    init();

    synchronize();

    uint32_t op = boot_get32();
    while (op != EXIT) {
        if (op == RUN) {
            run_prog();
        } else if (op == LIST) {
            // TODO
            // int n = get_connected();
        } else if (op == UPDATE) {
            // TODO
        }
        op = boot_get32();
    }

    clean_reboot();
}