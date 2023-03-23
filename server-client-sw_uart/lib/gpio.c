/*
 * Implement the following routines to set GPIO pins to input or output,
 * and to read (input) and write (output) them.
 *
 * DO NOT USE loads and stores directly: only use GET32 and PUT32
 * to read and write memory.  Use the minimal number of such calls.
 *
 * See rpi.h in this directory for the definitions.
 */
#include <stdio.h>

#include "rpi.h"
#include "rpi-interrupts.h"

// see broadcomm documents for magic addresses.
enum {
  GPIO_BASE = 0x20200000,
  gpio_fsel0 = GPIO_BASE,
  gpio_set0 = (GPIO_BASE + 0x1C),
  gpio_clr0 = (GPIO_BASE + 0x28),
  gpio_lev0 = (GPIO_BASE + 0x34),
  gpio_rise_edge0 = (GPIO_BASE + 0x4C),
  gpio_fall_edge0 = (GPIO_BASE + 0x58),
  gpio_event_detect0 = (GPIO_BASE + 0x40),
  gpudd = (GPIO_BASE + 0x94),
  gpuddCLK = (GPIO_BASE + 0x98)
};

//
// Part 1 implement gpio_set_on, gpio_set_off, gpio_set_output
//

// set <pin> to be an output pin.
//
// note: fsel0, fsel1, fsel2 are contiguous in memory, so you
// can (and should) use array calculations!
void gpio_set_output(unsigned pin) {
  gpio_set_function(pin, GPIO_FUNC_OUTPUT);
}

void gpio_set_function(unsigned pin, gpio_func_t function) {
  // 47 is the internal led pin
  if(pin >= 32 && pin != 47)
      return;

  // functions defined in gpio.h
  if (function < 0 || function > 7)
      return;

  // pg 93
  unsigned fsel_base = gpio_fsel0 + (pin / 10) * 4;
  unsigned bits = GET32(fsel_base);
  unsigned offset = (pin % 10) * 3;
  bits &= ~(0b111 << offset);
  bits |= (0b000 | function) << offset;
  PUT32(fsel_base, bits);
}

// set GPIO <pin> on.
void gpio_set_on(unsigned pin) {
  // 47 is the internal led pin
  if(pin >= 32 && pin != 47)
      return;

  // pg 90, 95
  PUT32(gpio_set0, 1 << pin);
}

// set GPIO <pin> off
void gpio_set_off(unsigned pin) {
  // 47 is the internal led pin
  if(pin >= 32 && pin != 47)
      return;

  // pg 90, 95
  PUT32(gpio_clr0, 1 << pin);
}

// set <pin> to <v> (v \in {0,1})
void gpio_write(unsigned pin, unsigned v) {
  // 47 is the internal led pin
  if(pin >= 32 && pin != 47)
      return;

  if (v)
    gpio_set_on(pin);
  else
    gpio_set_off(pin);
}

//
// Part 2: implement gpio_set_input and gpio_read
//

// set <pin> to input.
void gpio_set_input(unsigned pin) {
  gpio_set_function(pin, GPIO_FUNC_INPUT);
}

// return the value of <pin>
int gpio_read(unsigned pin) {
  // 47 is the internal led pin
  if(pin >= 32 && pin != 47)
      return -1;

  unsigned v = 0;

  // pg 96, 90
  v = GET32(gpio_lev0) & (1 << pin);
  return v > 0 ? DEV_VAL32(1) : DEV_VAL32(0);
}

// Lab 9 (interrupts)

static void or32(volatile void *addr, uint32_t val) {
    dev_barrier();
    put32(addr, get32(addr) | val);
    dev_barrier();
}
static void OR32(uint32_t addr, uint32_t val) {
    or32((volatile void*)addr, val);
}

// gpio_int_rising_edge and gpio_int_falling_edge (and any other) should
// call this routine (you must implement) to setup the right GPIO event.
// as with setting up functions, you should bitwise-or in the value for the 
// pin you are setting with the existing pin values.  (otherwise you will
// lose their configuration).  you also need to enable the right IRQ.   make
// sure to use device barriers!!
int is_gpio_int(unsigned gpio_int) {
  if (gpio_int < 0 || gpio_int > 3)
    return -1;
    
  OR32(Enable_IRQs_2, 1 << (gpio_int + 17)); // 17 = 49 - 32, see pg 113
  return 0;
}

// p97 set to detect rising edge (0->1) on <pin>.
// as the broadcom doc states, it  detects by sampling based on the clock.
// it looks for "011" (low, hi, hi) to suppress noise.  i.e., its triggered only
// *after* a 1 reading has been sampled twice, so there will be delay.
// if you want lower latency, you should us async rising edge (p99)
void gpio_int_rising_edge(unsigned pin) {
  // 47 is the internal led pin
  if(pin >= 32 && pin != 47)
      return;

  // pg 97
  unsigned bits = GET32(gpio_rise_edge0) | (1 << pin);
  PUT32(gpio_rise_edge0, bits);
}

// p98: detect falling edge (1->0).  sampled using the system clock.  
// similarly to rising edge detection, it suppresses noise by looking for
// "100" --- i.e., is triggered after two readings of "0" and so the 
// interrupt is delayed two clock cycles.   if you want  lower latency,
// you should use async falling edge. (p99)
void gpio_int_falling_edge(unsigned pin) {
  // 47 is the internal led pin
  if(pin >= 32 && pin != 47)
      return;

  // pg 98
  unsigned bits = GET32(gpio_fall_edge0) | (1 << pin);
  PUT32(gpio_fall_edge0, bits);
}

// p96: a 1<<pin is set in EVENT_DETECT if <pin> triggered an interrupt.
// if you configure multiple events to lead to interrupts, you will have to 
// read the pin to determine which caused it.
int gpio_event_detected(unsigned pin) {
  // 47 is the internal led pin
  if(pin >= 32 && pin != 47)
      return -1;

  // pg 96
  unsigned bits = GET32(gpio_event_detect0) & (1 << pin);
  return bits > 0 ? DEV_VAL32(1) : DEV_VAL32(0);
}

// p96: have to write a 1 to the pin to clear the event.
void gpio_event_clear(unsigned pin) {
  // 47 is the internal led pin
  if(pin >= 32 && pin != 47)
      return;

  // pg 96
  unsigned bits = GET32(gpio_event_detect0) | (1 << pin);
  PUT32(gpio_event_detect0, bits);
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
