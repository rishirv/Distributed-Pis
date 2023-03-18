// the sw uart should  print hello world 10x if it works.
#include "rpi.h"
#include "sw-uart.h"
#include "pi-esp.h"
#include "fast-hash32.h"

void notmain(void) {
    trace("about to use the sw-uart\n");
    trace("if your pi locks up, it means you are not transmitting\n");

    // turn off the hw UART so can use the same device.
    //uart_disable();

    gpio_set_input(21);
    gpio_set_pullup(21);
    gpio_set_output(23);
    
    /*
    // use pin 14 for tx, 15 for rx*/
    sw_uart_t u = sw_uart_init(23,21, 9600);

    char* cmd = kmalloc(sizeof(char) * 32);

    uint8_t* data = kmalloc(sizeof(uint8_t) * 32);

    uint8_t* send  = kmalloc(sizeof(uint8_t) * 30);
    char *value = "abcdefghijklmnopqrstuvwxyzabcd";
    memcpy(send, value, 30);
    
    // Test that you can send some command and 1 data packet
    send_cmd(&u, ESP_ACK,0b1010,0b1111,send,30);

    int i = sw_uart_get32B(&u,5000000, cmd);
    if (i != 33) {
        printk("Uh oh Seems we timed out on cmd byte i = %d\n", i);
    }

    i = sw_uart_get32B(&u,5000000, data);
    if (i != 33) {
        printk("Uh oh 2 Seems we timed out on data byte i = %d\n", i);
    }
     
    esp_cmnd_pckt_t * cmd_pkt = (esp_cmnd_pckt_t *) cmd;

    esp_pckt_t * data_pkt = (esp_pckt_t *) data;

    if (cmd_pkt->isCmd) {
      printk("CMD: isCmnd checks out\n");
    }
    if (cmd_pkt->cmnd == 0b1000){
      printk("CMD: command checks out\n");
    }
    if (cmd_pkt->esp_To == 0b1010){
      printk("CMD: TO checks out\n");
    }
    if (cmd_pkt->esp_From == 0b1111){
      printk("CMD: FROM checks out\n");
    }
    if (cmd_pkt->cksum == 0xffffffff) {
      printk("CMD: Checksum checks out!\n");
    }
    if (data_pkt->isCmd == 0) {
      printk("DATA: isCmnd checks out\n");      
    }
    if (data_pkt->nbytes == 30) {
      printk("DATA: nbytes checks out\n");      
    }
    if (data_pkt->esp_To == 0b1010){
      printk("DATA: TO checks out\n");
    }
    if (data_pkt->esp_From == 0b1111){
      printk("DATA: FROM checks out\n");
    }
    //printk("we got cmd = [%s]\n",cmd);
    //printk("we got data = [%s]\n", data);
    for (int i = 2; i < 32; i++) {
        printk("%c", data[i]);
    }
    trace("TRACE: done!\n");
}
