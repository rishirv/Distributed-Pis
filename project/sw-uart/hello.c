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
    char buff[32];

    sw_uart_put8(&u,'a');
    uint32_t start = timer_get_usec();
    for (int i = 0; i < 5; i++){
    buff[i] = sw_uart_get8(&u);
    }
    
    for(int i = 0; i< 5; i++){
        printk("%c\n",buff[i]);
    }
    uint32_t end = timer_get_usec();
    printk("%d\n", end-start);
    buff[6] = '\0';
    
    //delay_us(50000);
    printk("we got from esp [%s]\n",buff);
  //  printk("%c\n",e);
    //printk("%c\n",f);
    
    // reset to using the hardware uart.
 //   uart_init();
    trace("TRACE: done!\n");
}
