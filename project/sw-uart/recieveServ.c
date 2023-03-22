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
    uint8_t servIP = server_init();
    printk("server ip = %x\n",servIP);
}

void notmain(void) {
    init();
    trace("about to use the sw-uart\n");
    trace("if your pi locks up, it means you are not transmitting\n");

    // BIG DEAL TODO : to make the uart global I made a constants file: it just has a u header but 
    // it does not init it (cant do it dynamically from a header), so we must be sure to malloc and init this on startup. (why? well we need it global for the interrupt handler else its really hard)  
    
    //get what should be the client at fds 2
    fd fds = get_fd(0x2);
    while(has_msg(&fds)){
        fds=(get_fd(0xa));
    }

    msg_t* our_msg = get_msg(&fds);
    
    printk("\n this is msg: \n");
    for(int i = 0; i < 36; i++){
        printk("%c",our_msg->data[i]);
    }
    printk("\n");

    trace("TRACE: done!\n");
}