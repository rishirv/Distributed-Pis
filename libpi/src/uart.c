// implement:
//  void uart_init(void)
//
//  int uart_can_get8(void);
//  int uart_get8(void);
//
//  int uart_can_put8(void);
//  void uart_put8(uint8_t c);
//
//  int uart_tx_is_empty(void) {
//
// see that hello world works.
//
//
#include "rpi.h"

// called first to setup uart to 8n1 115200  baud,
// no interrupts.
//  - you will need memory barriers, use <dev_barrier()>
//
//  later: should add an init that takes a baud rate.
void uart_init(void) {
    dev_barrier();
    // enable gpio alt5 pin 14,15 
    gpio_set_function(GPIO_TX,GPIO_FUNC_ALT5);
    gpio_set_function(GPIO_RX,GPIO_FUNC_ALT5);

    // mem barrier 
    dev_barrier();
    // read mod write #1 on aux enable
    PUT32(AUX_ENB, GET32(AUX_ENB)|1);
    dev_barrier();
    PUT32(AUX_CNTL_REG,0);

    //disable interupts 
    PUT32(AUX_IER_REG,0);

    //set in 8 bit mode - see erata 
    PUT32(AUX_LCR_REG, 3);
    PUT32(0x20215050,0);
    //clear fifo 
    PUT32(AUX_IIR_REG,6);
    //configure....
    //disable flow - done with tx rx disable
    //configure baud 
    PUT32(AUX_BAUD_REG,270);
    //eable tx rx. 
    PUT32(AUX_CNTL_REG,3);
    dev_barrier();
}
int uart_tx_is_empty(void); 
// disable the uart.
void uart_disable(void) {
    uart_flush_tx();
 dev_barrier();
 PUT32(AUX_ENB, GET32(AUX_ENB) & ~0x1);
 dev_barrier();
}


// returns one byte from the rx queue, if needed
// blocks until there is one.
int uart_get8(void) {
    dev_barrier();
    while(!uart_has_data());
    return (int)GET32(AUX_IO_REG);
}

// 1 = space to put at least one byte, 0 otherwise.
int uart_can_put8(void) {
    return ((GET32(AUX_STAT_REG) >> 1) & 1);
}

// put one byte on the tx qqueue, if needed, blocks
// until TX has space.
// returns < 0 on error.
int uart_put8(uint8_t c) {
    dev_barrier();
    while(!uart_can_put8());
    PUT32(AUX_IO_REG,c);
    return 1;
}

// simple wrapper routines useful later.

// 1 = at least one byte on rx queue, 0 otherwise
int uart_has_data(void) {
    return (GET32(AUX_STAT_REG)  & 1);
}

// return -1 if no data, otherwise the byte.
int uart_get8_async(void) { 
    if(!uart_has_data())
        return -1;
    return uart_get8();
}

// 1 = tx queue empty, 0 = not empty.
int uart_tx_is_empty(void) {
    return ((GET32(AUX_STAT_REG) >> 9) & 1);
}

// flush out all bytes in the uart --- we use this when 
// turning it off / on, etc.
void uart_flush_tx(void) {

    while(!uart_tx_is_empty());
}
