#include "rpi.h"
#include "rpi-thread.h"
#include "gpio.h"
#include "timer-interrupt.h"

#define LED4 16
#define LED5 12
#define LED6 25
#define LED1 19
#define LED2 20
#define LED3 21
void print_thread(){
    printk("this is thread 1\n");
}

void ledInit(int led){
    gpio_set_output(led);
}

void delay_yield(uint32_t us){
    uint32_t s = timer_get_usec();
    while(1){
        uint32_t e = timer_get_usec();
        if((e-s) >= us){
            return;
        }else{
            rpi_yield();
        }
    }
}

void blink(void* led){
    trace("blink1\n");
    int i = 0;
    while(i < 2){
    gpio_set_on((int)led);
    delay_us(1000000);
    gpio_set_off((int)led);
    delay_us(1000000);
    i++;
    //fake_int();
    }
}

void blink2(void* led){
    trace("blink2 \n");
    trace("for real\n");
    int i = 0;
    while(i < 2){
    delay_us(1000000);
    gpio_set_on((int)led);
    delay_us(1000000);
    gpio_set_off((int)led);
    i++;
   // fake_int();
    }
}

void blink3(void* led){
    trace("blink3 \n");
        int i = 0;
    while(i < 2){
        delay_us(500000);
        gpio_set_on((int)led);
        delay_us(500000);
        gpio_set_off((int)led);
        i++;
    }
}

void interrupt_vector(unsigned pc){
    
    trace("\n XXX \n");
    dev_barrier();
    unsigned pending = GET32(IRQ_basic_pending);

    if((pending & RPI_BASIC_ARM_TIMER_IRQ) == 0) return;

    PUT32(arm_timer_IRQClear, 1);

    dev_barrier();

}

void notmain (void){
    printk("hello world\n");
   ledInit(LED1);
   ledInit(LED2);
   ledInit(LED3);
   ledInit(LED4);
   ledInit(LED5);
   ledInit(LED6);
    rpi_fork(blink,(int*)LED1);
    rpi_fork(blink2,(int*)LED2);
    rpi_fork(blink3,(int*)LED3);
    rpi_fork(blink,(int*)LED4);
    rpi_fork(blink2,(int*)LED5);
    rpi_fork(blink3,(int*)LED6);
    rpi_thread_start();
}
