#include "rpi.h"
#include "libc/fast-hash32.h"
#include "vector-base.h"
#include "sw-uart.h"
#include "constant.h"
#include "constants.h"
#include "pi-esp.h"
// #include "fds.h"

// sw_uart_t u;

void notmain() {
    // sw_uart_t u = sw_uart_init(4, 5, 9600);
    system_init(0);
    trace("about to use the sw-uart\n");
    trace("if your pi locks up, it means you are not transmitting\n");
    fd fds = get_fd(1);
    
    while(1) {
        // send_cmd(u,ESP_SEND_DATA,0x1,0x2,"ACK",4);
        trace("Waiting for program\n");
        while(Q_empty(&fileTable[1].msg_q));


        msg_t* buff = Q_pop(&fileTable[1].msg_q);
        // printk("Recieved: %s \n", buff->data);

        // int res = sw_uart_gets(&u, "START:", ":END", (void *) 0x80000, 8000);
        trace("Got program\n");

        memcpy((void *) 0x80000, buff->data, buff->totPckts * 30);
        // for (int i = 0; i < buff->totPckts * 30; i++)

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