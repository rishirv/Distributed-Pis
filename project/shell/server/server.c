#include "rpi.h"
#include "libc/fast-hash32.h"
#include "vector-base.h"
#include "sw-uart.h"
#include "get-code.h"
#include "constants.h"

static unsigned
has_data_timeout(unsigned timeout) {
    uint32_t start = timer_get_usec();
    while (1) {
        if (uart_has_data())
            return 1;
        uint32_t end = timer_get_usec();
        if ((end - start) >= timeout)
            return 0;
    }
}

static void wait_for_op(unsigned usec_timeout) {
    do {
        boot_put32(PI_READY);
    } while (has_data_timeout(usec_timeout) == 0);
}

void run_prog(void) {
    uint32_t nbytes = boot_get32();
    uint32_t crc = boot_get32();
    uint32_t n = boot_get32();
    uint8_t pis[n];
    for (int i = 0; i < n; i++) {
        pis[i] = uart_get8();
    }

    boot_put32(n);
    boot_put32(crc);

    uint32_t op = boot_get32();
    if (op != SEND_CODE) {
        boot_err(BOOT_ERROR, "Expected PUT_CODE");
        rpi_reboot();
    }

    uint32_t addr = 0x80000;

    for (int i = 0; i < nbytes; i++) {
        uint8_t byte = uart_get8();
        PUT8(addr + i, byte);
    }

    uint32_t *header = (void *)addr;
    // Confirm info in headers
    assert(header[0] = 0x12345678);
    assert(header[2] == 0x80000);

    boot_put32(CODE_GOT);

    // Should happen on client
    BRANCHTO(0x80010);

    boot_put32(DONE);

    (void)pis[0];
}

void notmain(void) {
    uart_init();

    wait_for_op(300 * 1000);
    unsigned op = boot_get32();
    if (op != UNIX_READY) {
        boot_err(BOOT_ERROR, "Expected UNIX_READY");
        rpi_reboot();
    }
    boot_put32(ACK);

    op = boot_get32();
    while (op != EXIT) {
        if (op == RUN) {
            run_prog();
            // boot_put32(1234);
        }
        op = boot_get32();
    }

    clean_reboot();
}