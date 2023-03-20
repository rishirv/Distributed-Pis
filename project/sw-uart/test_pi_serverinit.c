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
    
    server_init(&u);

    /*int c;
    // Promt esp to init itslef as an access point/server
    send_cmd(&u, ESP_SERVER_INIT, SELF, SELF, NULL, 0);
    if ((c = sw_uart_get8_timeout(&u,5000000)) == -1){
        printk("Uh oh Seems we timed out\n");
    } 
    printk("LSB we got from server esp IP: [%d]\n", c & 0xf);*/
    
    trace("TRACE: done!\n");
}
