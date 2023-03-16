#include "rpi.h"
#include "spi.h"
#include "sw-uart.h"
#include "pi-esp.h"
#include "fast-hash32.h"

// need an enum for the instruction codes

// need to figure out how we represent to/from

// will need a way to compute checksums

/*enum { 
    ESP_CLIENT_INIT         = 0b0001,
    ESP_SERVER_INIT         = 0b0010,
    ESP_SEND_DATA           = 0b0011,
    ESP_WIFI_CONNECT        = 0b0100,
    ESP_IS_CONNECTED        = 0b0101,
    ESP_GET_CONNECTED_LIST  = 0b0110,
    ESP_NOP                 = 0b0111,
    ESP_ACK                 = 0b1000,
    ESP_NOACK               = 0b1001,
};*/

uint8_t send_cmd(sw_uart_t *u, uint8_t cmd, uint8_t to, uint8_t from, const void *bytes, uint32_t nbytes) {
    // first check to make sure we have a valid cmd - make sure to malloc it
    esp_cmnd_pckt_t *header = kmalloc(sizeof(esp_cmnd_pckt_t));
    // zero out everything then set fields
    memset((char *)header, 0, sizeof(esp_cmnd_pckt_t));
    
    // Build the command packet header
    header->_sbz1 = 0b00;
    header->nbytes = CMD_NBYTES;
    header->isCmd = 0b1;
    header->esp_From = (from & 0xf);
    header->esp_To = (to & 0xf);
    
    // TODO: change this to compute a checksum of all the packets and their headers
    //uint32_t checksum = fast_hash32(bytes, nbytes);
    header->cksum =  0xffffffff;
    header->cmnd = 0xa;
    header->size = nbytes;
    // shouldn't need to zero out _sbz padding again?
    //header->_sbz[14] = 0xa;
    // For sanity checking
    /*trace("from = %d, to = %d\n", header->esp_From, header->esp_To);
    trace("checksum = %x\n", header->cksum);
    trace("cmnd = %d\n", header->cmnd);*/
    
    // send the packet!
    sw_uart_putPckt(u, header);
    printk("put header\n");

    // If nybtes is not 0, i.e. we have data to send too...create/send data packets!    
    uint32_t bytes_left = nbytes;
    char *data = (char *)bytes;
   
    // TODO: create all the data packets, store in an array, get cksum of that, THEN send! 
    while (bytes_left) {
        // Create data packet header
        uint8_t send_size = bytes_left >= 30 ? 30 : bytes_left % 30;
        esp_pckt_t *pckt = kmalloc(sizeof(esp_pckt_t));
        pckt->_sbz = 0;
        pckt->nbytes = send_size;
        pckt->isCmd = 0;    // this is a data packet!
        pckt->esp_From = (from & 0xf);
        pckt->esp_To = (to & 0xf);
        // Write a portion of data bytes to this packet
        memcpy(pckt->data, data, send_size);
        // send the data packet!
        sw_uart_putPckt(u, pckt);
        // update vars so we get next chunk of data
        bytes_left -= send_size;
        data += send_size;
        
    }
     
    //TODO wait on a timeout for an ack back from the pi.
    // can use a get32B cmnd and idk a timeout of like 1/2 second to start
    //
    // - eventually this would look like us having a fd that we can read from and 
    // not having to busy wait here. 
    
    //TODO if ack recieved then return 1,
    //If No-ACK or timeout then resend from the top, probably keep retrans counter and 
    //eventually return -1 if the retrans maxes out. (this could be like a pi disconnected all of a sudden)
    return 1;    

