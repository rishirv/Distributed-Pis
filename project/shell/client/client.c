#include "rpi.h"
#include "libc/fast-hash32.h"
#include "vector-base.h"
#include "sw-uart.h"
#include "constants.h"

sw_uart_t u;

void notmain() {
    sw_uart_t u = sw_uart_init(4, 5, 9600);

    while(1) {
        trace("Waiting for program\n");
        int res = sw_uart_gets(&u, "START:", ":END", (void *) 0x80000, 8000);
        trace("Got program\n");

        // verify recieved func is valid
        uint32_t *p = (void *) 0x80000;
        // magic cookie at offset 0.
        assert(p[0] == 0x12345678);
        // address to copy is at offset 2
        uint32_t addr = p[2];
        assert(addr == 0x80000);

        BRANCHTO(0x80010);

        uint32_t *nbytes = (void*) 0x1000000;
        uint8_t *buf = (void *) nbytes + sizeof(*nbytes);

        buf[*nbytes] = '\0';
        printk("%d %s", *nbytes, buf);

        printk("Job complete!\n");
    }
    clean_reboot();
}