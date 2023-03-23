#include "rpi.h"

uint32_t *nbytes;
uint8_t *buf;

int fake_put(int c) {
    *nbytes += 1;
    *(buf++) = c;
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
        nbytes = (void *) 0x1000000;
        *nbytes = 0;
        buf = (void *) nbytes + sizeof(*nbytes);

        notmain();

	// NB: cannot reboot!
        // rpi_reboot();
}
