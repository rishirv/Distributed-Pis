#include "pi-esp.h"
#include "sw-uart.h"
uint8_t send_cmd(uint8_t cmd, uint8_t to, const void *data, uint32_t nbytes){
    esp_cmnd_pckt* header = kmalloc(sizeof(esp_cmnd_pckt));
    header->isCmd = 1;
    header->esp_To = 0b1111;
    header->esp_From = 0b0101;
    header->nbytes = 11;
    header->_sbz1 = 0;
    
    //memset(header->_sbz,0,21);
    header->size = 0;
//    header->cmnd = cmd;
    header->cksum = (uint32_t)"chk";

    sw_uart_t u = sw_uart_init(23,21,9600);
    
    char* header_str = (char *) header;
    for (int i = 0; i < 32; i++){
        printk("%x ",header_str[i]);
    }

    sw_uart_putPckt(&u,header);
    return 1;
}
