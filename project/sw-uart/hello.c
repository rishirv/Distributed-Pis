// the sw uart should  print hello world 10x if it works.
#include "rpi.h"
#include "sw-uart.h"

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

  /*  for(int i = 0; i < 10; i++)
        sw_uart_putk(&u, "TRACE: sw_uart: hello world\n");
*/
    /*char* buff = kmalloc(sizeof(char) * 32);

    sw_uart_put8(&u,'a');
    if (sw_uart_get32B(&u,5000000, buff) == -1){
        printk("Uh oh Seems we timed out\n");
    } 
    printk("we got from esp [%s]\n",buff);

    // clearing the buffer
    //memset(0,buff,32);
    
   sw_uart_put8(&u,'b');
    if (sw_uart_get32B(&u,5000000,buff) == -1){
        printk("We timed out when expected, should wait about 5 seconds (i think)");
    }*/
    
   // char* buff = kmalloc(sizeof(char) * 15);
    //memset(buff,'a',13);
    //buff[14] = '\n';
    sw_uart_putk(&u,"abcdefghijklmnopqrstuvwxyzabcde");
    

    //printk("we got from esp (expect empty) [%s]\n",buff);

 //   uart_init();
    trace("TRACE: done!\n");
}
