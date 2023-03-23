#include "rpi.h"
#include "pi-sd.h"
#include "fat32.h"
#include "libc/fast-hash32.h"
#include "circular-queue.c"
#include "../lib/sw-uart.h"
#include "gpio.h"

const char *my_strstr(const char *haystack, const char *needle) {
    if (!*needle) {
        return haystack;
    }

    for (const char *h = haystack; *h; h++) {
        const char *n = needle;
        const char *p = h;

        while (*n && *p && *n == *p) {
            n++;
            p++;
        }

        if (!*n) {
            return h;
        }
    }

    return NULL;
}

void reverse(char* str, int length) {
    int start = 0;
    int end = length - 1;
    char temp;

    while (start < end) {
        temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

int int_to_str(int num, char* str) {
    int i = 0;
    int is_negative = 0;

    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return i;
    }

    if (num < 0) {
        is_negative = 1;
        num = -num;
    }

    while (num != 0) {
        str[i++] = (num % 10) + '0';
        num = num / 10;
    }

    if (is_negative) {
        str[i++] = '-';
    }

    str[i] = '\0';

    reverse(str, i);

    return i;
}

// this is the "main" function for the client.
void notmain(void) {
    // drivers
    uart_init();    
    kmalloc_init();
    pi_sd_init();
    // queue
    CircularQueue queue;
    initializeQueue(&queue, sizeof(pi_file_t), 10);
    // interrupt vector
    // extern uint32_t interrupt_vec[];
    // vector_base_set((void *)interrupt_vec);

    // while (1) {
    //     // TODO if there is a message from the server, read it and break
    // }

    // tx is 20, rx is 21
    sw_uart_t u = sw_uart_init(20, 21, 9600);
    // must set pullup!
    gpio_set_pullup(21);
    
    char buf[4096];

    // recieve the prog over sw-uart from the server
    int res = sw_uart_gets(&u, "START:", ":END", buf, sizeof buf - 1);

    // verify recieved func is valid
    uint32_t *p = (uint32_t *)buf;
    // magic cookie at offset 0.
    assert(p[0] == 0x12345678);
    // address to copy is at offset 2
    uint32_t addr = p[2];
    assert(addr == 0x80000);

    // temp — client-to-sever isn't working
    // tell the server to we've recieved the prog
    // printk("Sending ACK\n");
    // for (int i = 0; i < 100; i++) {
    //     sw_uart_put8(&u, 'X');
    // }

    // execute the prog
    void *ptr = (void *)0x80000;
    memcpy(ptr, buf, 4096);
    BRANCHTO(0x80010);

    printk("Job complete!\n");
    clean_reboot();

    return;
}