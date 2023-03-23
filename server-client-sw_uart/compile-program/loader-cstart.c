#include "rpi.h"

int fake_put(int c) {
    // MAJOR TODO: Write to buf instead and then send the data back to the server
    return c;
}

void _loader_cstart() {
        extern int __bss_start__, __bss_end__;
        void notmain();

        int* bss = &__bss_start__;
        int* bss_end = &__bss_end__;

        while( bss < bss_end )
                *bss++ = 0;

        rpi_putchar_set(fake_put);

        notmain();

	// NB: cannot reboot!
        // rpi_reboot();
}
