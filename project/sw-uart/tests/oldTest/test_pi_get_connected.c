#include "rpi.h"
#include "sw-uart.h"
#include "pi-esp.h"
#include "fast-hash32.h"

void notmain(void) {
    trace("about to use the sw-uart\n");
    trace("if your pi locks up, it means you are not transmitting\n");

    gpio_set_input(21);
    gpio_set_pullup(21);
    //TODO change back to 23
    gpio_set_output(23);
    
    /*
    // use pin 14 for tx, 15 for rx*/
    sw_uart_t u = sw_uart_init(23,21, 9600);
    
    uint8_t ip = server_init(&u);

    printk("LSB we got from server esp IP: [%d]\n", ip & 0xf);

    uint8_t *clients = (uint8_t *)kmalloc(sizeof(uint8_t)*MAX_NCLIENTS);
    
    printk("Found the following connected clients: \n");
    for (int i = 0; i < MAX_NCLIENTS; i++) {
        printk("Client %d IP: %d\n", i, clients[i] & 0xf);
    } 
    
    trace("TRACE: done!\n");
}
