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
}

void notmain(void) {
    init();
    trace("about to use the sw-uart\n");
    trace("if your pi locks up, it means you are not transmitting\n");

    // BIG DEAL TODO : to make the uart global I made a constants file: it just has a u header but 
    // it does not init it (cant do it dynamically from a header), so we must be sure to malloc and init this on startup. (why? well we need it global for the interrupt handler else its really hard)  
    
    // okayyy lets say to = 0xa and from  = 0xa (just to make this sane for myself) 
    // so grab fds from 0xa
    fd fds = get_fd(0xa);
    printk("here is our fds in rec: %x\n",fds);
    // alright easy thing here is to send a well formed message to the pi: then have the pi send that msg right back

    //send_cmd(u,0b1111,0b1010,0b1010,"hello",6);
   // send_cmd(u,0b1111,0b1010,0b1010,"HELLO WORLD FROM PI HERE IS SOME PACKETS ITS GONNA BE A LOT SO JUST HANG IN THERE",82);
    
    while(has_msg(&fds)){
        // okay this isnt great as we kmalloc everytime but eventually itll at least be interrupts
        //recieveMsgHandler();
     //   printk("returned from msg handler\n");
        fds=(get_fd(0xa));
    }

    msg_t* our_msg = get_msg(&fds);
    
    printk("\n this is msg: \n");
    for(int i = 0; i < 36; i++){
        printk("%c",our_msg->data[i]);
    }
    printk("\n");
    //printk("This is msg: %s \n",our_msg->data);
    // okay now we will sit and hit the recieve msg until we see somethign in the queue
    trace("TRACE: done!\n");
}
