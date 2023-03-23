// the sw uart should  print hello world 10x if it works.
#include "rpi.h"
#include "constants.h"
#include "sw-uart.h"
#include "pi-esp.h"
#include "fast-hash32.h"
#include "rpi-interrupts.h"
#include "constants.h"


void init(){
    // repetitive, sw uart init does it , but here for sanity 
    gpio_set_input(21);
    gpio_set_pullup(21);
    gpio_set_output(23);
    

    u = (sw_uart_t*) kmalloc(sizeof(sw_uart_t));
    *u = sw_uart_init(23,21,9600);
    char* buff = kmalloc(sizeof(char) * 32);
    
    init_fileTable();
    int_init();
    gpio_int_falling_edge(21);
    system_enable_interrupts();
    
   // printk("init server:\n");
  //  send_cmd(u,ESP_SEND_DATA,0xf,0xf,"DOES THIS WORK",25);
    printk("about to inti server\n");
    int servIP = server_init();
  //  while(servIP == -1) servIP = server_init();
    printk("server ip = %x\n",servIP);
}
void notmain(void) {
    init();
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
