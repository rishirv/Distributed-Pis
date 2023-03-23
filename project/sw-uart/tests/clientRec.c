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
    //char* buff = kmalloc(sizeof(char) * 32);
    
    init_fileTable();
    int_init();
    gpio_int_falling_edge(21);
    system_enable_interrupts();
    
    // basically loop until connect to wifi returns something other than -1 
    while(connect_to_wifi(u) == -1){
        delay_us(100000);
        printk("connecting... \n");
    }
    printk("connected to wifi\n");
    // then take what it returns and set our ip equal to it 

    // then make a call to get server ip to get the server ip. 
    // just hardcode it for now
    
}

void notmain(void) {
    init();
    trace("about to use the sw-uart\n");
    trace("if your pi locks up, it means you are not transmitting\n");

    // BIG DEAL TODO : to make the uart global I made a constants file: it just has a u header but 
    // it does not init it (cant do it dynamically from a header), so we must be sure to malloc and init this on startup. (why? well we need it global for the interrupt handler else its really hard)  
    
    // okayyy lets say to = 0xa and from  = 0xa (just to make this sane for myself) 
    // so grab fds from 0xa
    fd fds = get_fd(1);
    
    send_cmd(u,ESP_SEND_DATA,0x1,0x2,"ACK",4);
    while(!Q_empty(&fileTable[1].msg_q));


    msg_t* buff = Q_pop(&fileTable[1].msg_q);
    printk("Recieved: %s \n",buff->data);


    trace("TRACE: done!\n");
}
