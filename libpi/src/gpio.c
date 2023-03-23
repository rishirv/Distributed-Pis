/*
 * Implement the following routines to set GPIO pins to input or output,
 * and to read (input) and write (output) them.
 *
 * DO NOT USE loads and stores directly: only use GET32 and PUT32
 * to read and write memory.  Use the minimal number of such calls.
 *
 * See rpi.h in this directory for the definitions.
 */
#include "rpi.h"
#include "timer-interrupt.h"

// see broadcomm documents for magic addresses.
enum {
    GPIO_BASE = 0x20200000,
    gpio_set0  = (GPIO_BASE + 0x1C),
    gpio_clr0  = (GPIO_BASE + 0x28),
    gpio_lev0  = (GPIO_BASE + 0x34),
    gpudd = (GPIO_BASE + 0x94),
    gpuddCLK = (GPIO_BASE + 0x98)
};



void gpio_set_function(unsigned pin, gpio_func_t function){
    if (pin >= 32 || function < 0 || function > 7) return;

    unsigned  fSEL =  GPIO_BASE + (4 *(pin / 10));
    unsigned shift = (pin % 10)*3;
    unsigned mask = 7 << shift;
    unsigned value = function << shift;
    
    PUT32(fSEL,(GET32(fSEL) & ~mask)|value);
}
//
// Part 1 implement gpio_set_on, gpio_set_off, gpio_set_output
//

// set <pin> to be an output pin.
//
// note: fsel0, fsel1, fsel2 are contiguous in memory, so you
// can (and should) use array calculations!
void gpio_set_output(unsigned pin) {
     gpio_set_function(pin,GPIO_FUNC_OUTPUT);
   /* if(pin >= 32)
        return;
    
    //get register for reading
    unsigned  fSEL =  GPIO_BASE + (4 *(pin / 10));
    //bitmask correct pins 
    unsigned shift = (pin % 10)*3;
    unsigned mask = 7 << shift;
    unsigned value = 0x1 << shift;
    
    PUT32(fSEL,(GET32(fSEL) & ~mask)|value);

  // implement this
  // use <gpio_fsel0>*/
}

// set GPIO <pin> on.
void gpio_set_on(unsigned pin) {
    //system_disable_interrupts();
    if(pin >= 32)
        return;
    PUT32(gpio_set0,(0x1)<<pin);
  // implement this
  // use <gpio_set0>
  //system_enable_interrupts();
}

// set GPIO <pin> off
void gpio_set_off(unsigned pin) {
    //system_disable_interrupts();
    if(pin >= 32)
        return;
    PUT32(gpio_clr0,(0x1)<<pin);
  // implement this
  // use <gpio_clr0>
  //system_enable_interrupts();
}

// set <pin> to <v> (v \in {0,1})
void gpio_write(unsigned pin, unsigned v) {
    if(v)
        gpio_set_on(pin);
    else
        gpio_set_off(pin);
}

//
// Part 2: implement gpio_set_input and gpio_read
//

// set <pin> to input.
void gpio_set_input(unsigned pin) {
     gpio_set_function(pin,GPIO_FUNC_INPUT);
   /* if(pin >= 32)
   if(pin >= 32)
        return;
    
    //get register for reading
    unsigned  fSEL =  GPIO_BASE + (4 *(pin / 10));
    //bitmask correct pins 
    //bitmask correct pins 
    unsigned shift = (pin % 10)*3;
    unsigned mask = 7 << shift;
    unsigned value = 0x0 << shift;
    
    PUT32(fSEL,(GET32(fSEL) & ~mask)|value);*/

}

// return the value of <pin>
int gpio_read(unsigned pin) {
    if (pin >= 32 && pin != 47) return -1;
    unsigned mask =  0x1 << pin;
    return DEV_VAL32((GET32(gpio_lev0) & mask) >> pin) ; 
}

void gpio_set_pullup(unsigned pin){
    if (pin >= 32){
        return;
    }
    // initializes to pulldown bcm pg 101
   PUT32(gpudd,0x2); 
    // block 150us
    for(int wait = 0; wait < 150; wait++) delay_cycles(1);
    
    // write to clk the pins we are on 
     uint32_t mask = 0x1 << pin; 
    PUT32(gpuddCLK, (GET32(gpuddCLK) & ~mask) | 0x1<<pin);
   // PUT32(gpuddCLK,0x1<<pin);

    for(int wait = 0; wait < 150; wait++) delay_cycles(1);
    PUT32(gpudd, 0); 
    PUT32(gpuddCLK,0);
}

void gpio_set_pulldown(unsigned pin){
    if (pin >= 32){
        return;
    }
    // initializes to pulldown bcm pg 101
   PUT32(gpudd,0x1); 
    // block 150us
    for(int wait = 0; wait < 150; wait++) delay_cycles(1);
    
    // write to clk the pins we are on 
     uint32_t mask = 0x1 << pin; 
    PUT32(gpuddCLK, (GET32(gpuddCLK) & ~mask) | 0x1<<pin);
   // PUT32(gpuddCLK,0x1<<pin);

    for(int wait = 0; wait < 150; wait++) delay_cycles(1);
    PUT32(gpudd, 0); 
    PUT32(gpuddCLK,0);
}

void gpio_pud_off(unsigned pin){
    if (pin >= 32){
        return;
    }
    // initializes to pulldown bcm pg 101
   PUT32(gpudd,0x0); 
    // block 150us
    for(int wait = 0; wait < 150; wait++) delay_cycles(1);
    
    // write to clk the pins we are on 
     uint32_t mask = 0x1 << pin; 
    PUT32(gpuddCLK, (GET32(gpuddCLK) & ~mask) | 0x1<<pin);
     for(int wait = 0; wait < 150; wait++) delay_cycles(1);
    PUT32(gpudd, 0); 
    PUT32(gpuddCLK,0);

}

/*
int gpio_has_interrupt(void){
    dev_barrier();
    int val = ((GET32(IRQ_pending_2) >> (GPIO_INT0%32)) & 1);
    dev_barrier();
    return val;
}

void gpio_int_rising_edge(unsigned pin){
    dev_barrier();
    PUT32(0x2020004C,GET32(0x2020004C) | 1 << pin);
    dev_barrier();
}

void gpi_int_falling_edge(unsigned pin){
    dev_barrier();
    PUT32(0x20200058,GET32(0x20200058) | 1 << pin);
    dev_barrier();
}

void gpio_enable_hi_int(unsigned pin){
    dev_barrier();
    PUT32(0x20200064,GET32(0x20200064) | 1<< pin );
    dev_barrier();
}

void gpio_int_low(unsigned pin){
    dev_barrier();
    PUT32(0x20200070, GET32(0x20200070) | 1 << pin);
    dev_barrier();
}

int gpio_event_detected(unsigned pin){
    dev_barrier();
    int val = (GET32(0x20200040) >> pin & 1);
    dev_barrier();
    return val;
}

void gpio_event_clear(unsigned pin){
    dev_barrier();
    PUT32(0x20200040, 1 << pin);
    dev_barrier();
}*/
