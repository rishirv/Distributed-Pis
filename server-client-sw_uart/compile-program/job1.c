#include "rpi.h"

void notmain(void) {
	// NB: we can't do this b/c the shell already initialized and resetting
	// uart may reset connection to Unix.
	// uart_init();

	// if not working, try just printing characters.
#if 0
	uart_putc('h');
	uart_putc('e');
	uart_putc('l');
	uart_putc('l');
	uart_putc('o');

	uart_putc(' ');

	uart_putc('w');
	uart_putc('o');
	uart_putc('r');
	uart_putc('l');
	uart_putc('d');

	uart_putc('\n');
#endif
	printk("hello world from address %p\n", (void*)notmain);
	printk("blinking 10 times...\n");

	int led = 0;
    gpio_set_output(led);
    for(int i = 0; i < 10; i++) {
        gpio_set_on(led);
        delay_cycles(1000000);
        gpio_set_off(led);
        delay_cycles(1000000);
    }

	printk("done blinking\n");

	return;

	// NB: this is supposed to be a thread_exit().  calling reboot will
	// kill the pi.
	// clean_reboot();
}
