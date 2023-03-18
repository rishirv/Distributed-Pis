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
    //TODO change back to 23
    gpio_set_output(23);
    
    /*
    // use pin 14 for tx, 15 for rx*/
    sw_uart_t u = sw_uart_init(23,21, 9600);

    char* buff = kmalloc(sizeof(char) * 32);
    
    send_cmd(&u,0b1111,0b1010,0b1010,"HELLO WORLD FROM PI HERE IS SOME PACKETS ITS GONNA BE A LOT SO JUST HANG IN THERE",82);
    trace("TRACE: done!\n");
}
