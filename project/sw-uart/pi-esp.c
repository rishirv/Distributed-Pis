#include "rpi.h"
#include "spi.h"
#include "sw-uart.h"
#include "pi-esp.h"
#include "fast-hash32.h"

/* Commands encoded like so:
enum { 
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

/* Prompt the esp to init itself as a station aka client in its setup
Note: might not use, might just flash client code to dedicated client esps */
uint8_t client_init(void) {
    return 1;
}

/* Prompt the esp to init itself as an access point aka server in its setup
Note: might not use, might just flash server code to dedicated server esp */
uint8_t server_init(void) {
    return 1;
}

/* Send command and data from pi to esp in 32 byte packets with the following form:
    Packet Headers (2 bytes) --> On every 32byte packet
    4 bits: To
    4 bits: From
    1 bit: isCmd
    5 bits: nbytes (in packet)
    2 bits: SBZ
    If isCmd: remaining bytes used for
    4 bytes --> Size of data being sent (2GB)
    1 bytes --> The actual Cmd (like connect to wifi or whatever)
    4 bytes --> Check sum of all the data

    Top 2 bytes of 32 byte packet:

    bits:    15 14 13 12 11 10 9   8   7 6 5 4 3 2 1 0
             | sbz |   nbytes   |isCmd| from  |  to   |
    byte:               31                 30

    Remaining bytes 29-0 are either data or the following if a command packet:
    
    byte:    29 28 27 26  25  24 23 22 21 20 ... 0
             | totalsize |cmd| checksum  |        |
*/
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
    // get checksum AFTER gathering all the data packets 
    header->cmnd = cmd;
    header->size = nbytes;
    // shouldn't need to zero out _sbz padding again?
    //header->_sbz[14] = 0xa;
    // For sanity checking
    /*trace("from = %d, to = %d\n", header->esp_From, header->esp_To);
    trace("checksum = %x\n", header->cksum);
    trace("cmnd = %d\n", header->cmnd);*/
    
    // If nybtes is not 0, i.e. we have data to send too...create/send data packets!    
    uint32_t bytes_left = nbytes;
    char *data = (char *)bytes;
    uint32_t npckts = (nbytes/DATA_NBYTES) + ((nbytes % DATA_NBYTES) > 0);
    
    esp_pckt_t *pckts = NULL;
    if (npckts) 
        pckts = kmalloc(sizeof(esp_pckt_t) * npckts); 
   
    // create all the data packets, store in an array, get cksum of that, THEN send!
    int curr_pkt = 0;
    while (bytes_left) {
        // Create data packet header
        uint8_t send_size = bytes_left >= 30 ? 30 : bytes_left % 30;
        esp_pckt_t pckt = {
            ._sbz = 0,
            .nbytes = send_size,
            .isCmd = 0,    // this is a data packet!
            .esp_From = (from & 0xf),
            .esp_To = (to & 0xf),
        };
        // Write a portion of data bytes to this packet
        memcpy(pckt.data, data, send_size);
        // store data packet in pckts array to send later
        pckts[curr_pkt] = pckt; 
        // update vars so we get next chunk of data
        bytes_left -= send_size;
        data += send_size;
        curr_pkt++;
    }

    // compute checksum over all the data packets and their headers 
    uint32_t checksum = fast_hash32(pckts,sizeof(esp_pckt_t) * PKT_NBYTES);
    header->cksum = checksum;

    // send the command packet!
    sw_uart_putPckt(u, header);

    // Now send all the data packets one by one
    for (int i = 0; i < npckts; i++)  {
        sw_uart_putPckt(u, &pckts[i]);
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
}

/* Receive data from esp by transferring 0's over SPI. Returns a buffer with the esp's
response or null if unsuccessful.*/
uint8_t receive_data_nbytes(void) {
    return 1;
}

// For Client: Prompt client esp to connect to the server via Wifi.begin()
uint8_t connect_to_wifi(void) {
    // Call send_cmd with ESP_WIFI_CONNECT
    // If return == -1, update fd field with no_ack?
    // otherwise, update fd field with ack?
    return 1;
}

// For Client: Returns whether or not this client pi's esp is connected to the server
uint8_t is_connected(void) {
    // Call send_cmd with ESP_IS_CONNECTED
    // If return == -1, update fd field with no_ack
    // otherwise, update fd field with ack 
    return 1;
}

// For Server: Obtains a list of clients currently connected to server
uint8_t *get_connected(void) {
    return NULL;
}
