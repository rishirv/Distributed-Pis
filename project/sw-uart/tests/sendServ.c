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

    printk("starting\n");
    //system_init(1);
    init();
    trace("about to use the sw-uart\n");
    trace("if your pi locks up, it means you are not transmitting\n");

 
    //hardcoded for me to work on 2 esps and 1 pi
   /* int j = 0;
    while(1){
        delay_us(10000000);
        j++;
        send_cmd(u,ESP_SEND_DATA,2,0x1,"SENT DATA FROM SERVER",25);
        printk("sent data %d\n",j);
    }
*/
   
    uint8_t pis[16];
    memset(pis,0,16);
    
    while(1){
    for(int i = 0; i < 16; i++){
        if(Q_empty(&fileTable[i].msg_q)){
            continue;}
        //we have a message
        msg_t* msg = Q_pop(&fileTable[i].msg_q);
        //char* msg = (char*)get_msg(&fds);
        if(strcmp(msg->data,"ACK")==0) {
            printk("Got ACK from %d \n",i);
            for(int k = 0 ; k < 4 ; k++){

                //send_cmd(u,ESP_SEND_DATA,2,0x1,"SENT DATA FROM SERVER",25);
                sendProgram(i,"HELLO FROM BEN ON SERVER",26);
                delay_us(100000);
            }
            printk("sent msg\n");
            //msg = NULL;
            break;
        }

        else printk("got %d from pi NOT expected\n",i);
        }
    }

    trace("TRACE: done!\n");
}
