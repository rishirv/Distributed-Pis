/*****************************************************************
 * bootloader implementation.  all the code you write will be in
 * this file.  <get-code.h> has useful helpers.
 *
 * if you need to print: use boot_putk.  only do this when you
 * are *not* expecting any data.
 */
#include "rpi.h"
#include "memmap.h"
#include "get-code.h"

// wait until:
//   (1) there is data (uart_has_data() == 1): return 1.
//   (2) <timeout> usec expires, return 0.
//
// look at libpi/staff-src/timer.c
//   - call <timer_get_usec()> to get usec
//   - look at <delay_us()> : for how to correctly 
//     wait for <n> microseconds given that the hardware
//     counter can overflow.
static unsigned 
has_data_timeout(unsigned timeout) {
    uint32_t start = timer_get_usec();
    while(1) {
        if (uart_has_data())
            return 1;
        uint32_t end = timer_get_usec();
        if ((end - start) >= timeout)
            return 0;
    }
}

// iterate:
//   - send GET_PROG_INFO to server
//   - call <has_data_timeout(<usec_timeout>)>
//       - if =1 then return.
//       - if =0 then repeat.
//
// NOTE:
//   1. make sure you do the delay right: otherwise you'll
//      keep blasting <GET_PROG_INFO> messages to your laptop
//      which can end in tears.
//   2. rember the green light that blinks on your ttyl device?
//      Its from this loop (since the LED goes on for each 
//      received packet)
static void wait_for_data(unsigned usec_timeout) {
    do {
        boot_put32(GET_PROG_INFO);
    } while (has_data_timeout(usec_timeout) == 0);
}

// IMPLEMENT this routine.
//
// Simple bootloader: put all of your code here.
int get_code(uint32_t *code_addr) {
    // 0. keep sending GET_PROG_INFO every 300ms until 
    // there is data: implement this.
    wait_for_data(300 * 1000);

    /****************************************************************
     * Add your code below: 2,3,4,5,6
     */
    uint32_t addr = 0;

    // 2. expect: [PUT_PROG_INFO, addr, nbytes, cksum] 
    //    we echo cksum back in step 4 to help debugging.
    unsigned op = boot_get32();
    if (op != PUT_PROG_INFO) {
        boot_err(BOOT_ERROR, "Expected PUT_PROG_INFO");
        return 0;
    }
    addr = boot_get32();
    unsigned nbytes = boot_get32();
    unsigned crc = boot_get32();

    // 3. If the binary will collide with us, abort with a BOOT_ERROR. 
    // 
    //    for today: assume that code must be below where the 
    //    booloader code gap starts.  make sure both the start and 
    //    end is below <get_code>'s address.
    // 
    //    more general: use address of PUT32 and __PROG_END__ to detect: 
    //    see libpi/memmap and the memmap.h header for definitions.
    unsigned end = addr + nbytes;
    if ((addr < (uintptr_t) __code_start__) || (end >= (uintptr_t) PUT32) || (end >= (uintptr_t) __prog_end__)) {
        boot_err(BOOT_ERROR, "Code will collide with the bootloader");
        return 0;
    }

    // 4. send [GET_CODE, cksum] back.
    boot_put32(GET_CODE);
    boot_put32(crc);

    // 5. we expect: [PUT_CODE, <code>]
    //  read each sent byte and write it starting at 
    //  <addr> using PUT8
    //
    // common mistake: computing the offset incorrectly.
    op = boot_get32();
    if (op != PUT_CODE) {
        boot_err(BOOT_ERROR, "Expected PUT_CODE");
        return 0;
    }

    for (int i = 0; i < nbytes; i++) {
        uint8_t byte = uart_get8();
        PUT8(addr + i, byte);
    }

    // 6. verify the cksum of the copied code using:
    //         boot-crc32.h:crc32.
    //    if fails, abort with a BOOT_ERROR.
    if (crc32((void *) addr, nbytes) != crc) {
        boot_err(BOOT_ERROR, "Checksum mismatch");
        return 0;
    }

    // 7. send back a BOOT_SUCCESS!
    boot_putk("Rishi Verma: success: Received the program with UART!");

    // woo!
    boot_put32(BOOT_SUCCESS);

    // We used to have these delays to stop the unix side from getting 
    // confused.  I believe it's b/c the code we call re-initializes the 
    // uart.  Instead we now flush the hardware tx buffer.   If this
    // isn't working, put the delay back.  However, it makes it much faster
    // to test multiple programs without it.
    // delay_ms(500);
    uart_flush_tx();

    *code_addr = addr;
    return 1;
}
