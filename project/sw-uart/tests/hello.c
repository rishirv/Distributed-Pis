// the sw uart should  print hello world 10x if it works.
#include "rpi.h"
#include "sw-uart.h"
#include "pi-esp.h"
#include "fast-hash32.h"
#include "constants.h"
void notmain(void) {
    trace("about to use the sw-uart\n");
    trace("if your pi locks up, it means you are not transmitting\n");

    // turn off the hw UART so can use the same device.
    //uart_disable();

    gpio_set_input(21);
    gpio_set_pullup(21);
    //TODO change back to 23
    gpio_set_output(23);
    
    /*
    // use pin 14 for tx, 15 for rx*/
    //sw_uart_t u = sw_uart_init(23,21, 9600);
    
    // BIG DEAL TODO : to make the uart global I made a constants file: it just has a u header but 
    // it does not init it (cant do it dynamically from a header), so we must be sure to malloc and init this on startup. (why? well we need it global for the interrupt handler else its really hard)  
    u = (sw_uart_t*) kmalloc(sizeof(sw_uart_t));
    *u = sw_uart_init(23,21,9600);
    char* buff = kmalloc(sizeof(char) * 32);

    // sends command to connect
    //send_cmd(u,0b0100,0b1010,0b1010,NULL,0);
    uint8_t success = connect_to_wifi();
    if(success) printk("Success connected to wifi\n");
    trace("TRACE: done!\n");
}
