// the sw uart should  print hello world 10x if it works.
#include "rpi.h"
#include "constants.h"
#include "sw-uart.h"
#include "pi-esp.h"
#include "fast-hash32.h"
#include "rpi-interrupts.h"
#include "constants.h"


void notmain(void) {
    system_init(1);
    printk("succesfully got system inited\n");
    trace("about to use the sw-uart\n");
    trace("if your pi locks up, it means you are not transmitting\n");
   
    int connections = 0;
    uint8_t* buff = (uint8_t*)kmalloc(sizeof(uint8_t*)*16);
    while(1){
       connections = get_connected(buff);
       if(connections > 0){
           printk("Printing connections:\n");
           for(int i =0; i<connections;i++){
               printk("%d, ",buff[i]);
           }
           printk("\n\n");
       }
        delay_us(1000000);
    }

    trace("TRACE: done!\n");
}
