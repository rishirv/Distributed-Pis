// the sw uart should  print hello world 10x if it works.
#include "rpi.h"
#include "sw-uart.h"
#include "pi-esp.h"
#include "fast-hash32.h"

void notmain(void) {
    trace("about to use the sw-uart\n");
    trace("if your pi locks up, it means you are not transmitting\n");

    // turn off the hw UART so can use the same device.
    //uart_disable();

    gpio_set_input(21);
    gpio_set_pullup(21);
    gpio_set_output(23);
    
    /*
    // use pin 14 for tx, 15 for rx*/
    sw_uart_t u = sw_uart_init(23,21, 9600);
    
    // Test that you can send the following command with no data
    send_cmd(&u, ESP_ACK,0b1010,0b1111,NULL,0);

    if (sw_uart_get32B(&u,5000000, buff) == -1){
        printk("Uh oh Seems we timed out\n");
    } 
    printk("we got from esp [%s]\n",buff);
    trace("TRACE: done!\n");
}
