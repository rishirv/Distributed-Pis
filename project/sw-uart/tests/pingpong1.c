// the sw uart should  print hello world 10x if it works.
#include "rpi.h"
#include "constants.h"
#include "sw-uart.h"
#include "pi-esp.h"
#include "fast-hash32.h"
#include "rpi-interrupts.h"
#include "constants.h"


void notmain(void) {
    printk("about to inti server\n");
    system_init(1);
  //  while(servIP == -1) servIP = server_init();
    printk("finsihed init\n");

    trace("about to use the sw-uart\n");
    trace("if your pi locks up, it means you are not transmitting\n");
   
    //Assuming just one client connected
    
    // get the clients fd
    uint8_t* buff = (uint8_t*)kmalloc(sizeof(uint8_t*)*16);
    int connections = 0;
    while(connections == 0){
        connections = get_connected(buff);
        printk("no connections found delaying\n");
        delay_us(1000000);
    }
    printk("connection found checking..\n");

    if (connections > 1) printk("unexpected extra connection\n");
    if(buff[0] != 2) printk("unexpected ip value %d\n",buff[0]);
    
    printk("connection checked out initiating ping pong\n");
    // okay connection checks out lets roll
    fd fds = get_fd(buff[0]);
    int i = 0;
    while(i < 1000){
        printk("i : %d",i);
       sendProgram(buff[0],&i,4) ;
       while(has_msg(&fds)) fds = get_fd(buff[0]);
       int* msg = (int*)get_msg(&fds);
       if (*msg == i+1) i+= 2;
       else panic("msg unexpected %d expected %d",*msg, i+1);
    }
    trace("TRACE: done!\n");
}
