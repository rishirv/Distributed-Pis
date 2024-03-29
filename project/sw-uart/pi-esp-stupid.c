#include "rpi.h"
#include "spi.h"
#include "sw-uart.h"
#include "pi-esp.h"
#include "fast-hash32.h"

// need an enum for the instruction codes

// need to figure out how we represent to/from

// will need a way to compute checksums

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

uint8_t spi_chip = 0; // just use chip 0 always

/* Prompt the esp to init itself as a station aka client in its setup
Note: might not use, might just flash client code to dedicated client esps */
uint8_t client_init(void) {
    uint8_t rx[32], tx[32];
    tx[0] = ESP_CLIENT_INIT;
    // not sure about the clock divider but this is what Dawson used
    // see pin_init from lab 16
    spi_t s = spi_n_init(spi_chip, 20);
    spi_n_tranfer(s, rx, tx, 1);
    /* TODO: either get status (i.e. success/failure) from status register, see lines 48-50:
    https://github.com/esp8266/Arduino/blob/master/libraries/SPISlave/examples/SPISlave_Test/SPISlave_Test.ino
    OR poll (transfer 0s) until you get a response from esp */
}

/* Prompt the esp to init itself as an access point aka server in its setup
Note: might not use, might just flash server code to dedicated server esp */
uint8_t server_init(void) {
    uint8_t rx[32], tx[32];
    tx[0] = ESP_SERVER_INIT;
    // not sure about the clock divider but this is what Dawson used
    // see pin_init from lab 16
    spi_t s = spi_n_init(spi_chip, 20);
    spi_n_tranfer(s, rx, tx, 1);
    // TODO: either get status (i.e. success/failure) from status register
    // or poll (transfer 0s) until you get a response from esp
}

uint8_t convert_ip(uint32_t ip) {
    // encode a 32bit ip address as its lower 4 bits
    return (uint8_t)((ip << 28) >> 28);
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

    bits:    15 14 13 12 11   10  9 8 7 6 5 4 3 2 1 0
             | nbytes      |isCmd| from  | to    |sbz|
    byte:               31                 30

    Remaining bytes 29-0 are either data or the following if a command packet:
    
    byte:    29 28 27 26  25  24 23 22 21 20 ... 0
             | totalsize |cmd| checksum  |        |
*/
uint8_t send_cmd(uint8_t cmd, uint32_t to, uint32_t from, const void *bytes, uint32_t nbytes) {
    // 1. build command header, will decompose later!
    uint8_t tx[32];
    // msb to lsb: nbytes (0 for cmd packet), isCmd bit, from, to, 2 0s padding
    uint16_t hdr = (0 << 11) | (1 << 10) | ((from & 0xf) << 6) | ((to & 0xf) << 2);
    tx[0] = (hdr >> 8) & 0xff;
    tx[1] = hdr & 0xff;
    
    // 2. fill in total size, cmd, checksum
    uint32_t checksum = fast_hash32(bytes, nbytes);
    tx[2] = (nbytes >> 24) & 0xff;
    tx[3] = (nbytes >> 16) & 0xff;
    tx[4] = (nbytes >> 8) & 0xff;
    tx[5] = nbytes & 0xff;
    tx[6] = cmd;
    tx[7] = (checksum >> 24) & 0xff;
    tx[8] = (checksum >> 16) & 0xff;
    tx[9] = (checksum >> 8) & 0xff;
    tx[10] = checksum & 0xff;

    // 3. figure out how many data packets we need to send before firing off cmd
    // nbytes might not be a mult of 30 (32 - HEADER_SIZE)
    uint32_t rem = nbytes % 30;
    uint32_t npackets = (nbytes/30) + rem;
    
    // 4. send cmd packet!
    sw_uart_putk(uart, tx);
}

/* Receive data from esp by transferring 0's over SPI. Returns a buffer with the esp's
response or null if unsuccessful.*/
uint8_t receive_data_nbytes(void);

// For Client: Prompt client esp to connect to the server via Wifi.begin()
uint8_t connect_to_wifi(void) {
    uint8_t rx[32], tx[32];
    tx[0] = ESP_WIFI_CONNECT;
    // not sure about the clock divider but this is what Dawson used
    // see pin_init from lab 16
    spi_t s = spi_n_init(spi_chip, 20);
    spi_n_tranfer(s, rx, tx, 1);
    // TODO: either get status (i.e. success/failure) from status register
    // or poll (transfer 0s) until you get a response from esp
}

// For Client: Returns whether or not this client pi's esp is connected to the server
uint8_t is_connected(void) {
    uint8_t rx[32], tx[32];
    tx[0] = ESP_WIFI_CONNECT;
    // not sure about the clock divider but this is what Dawson used
    // see pin_init from lab 16
    spi_t s = spi_n_init(spi_chip, 20);
    spi_n_tranfer(s, rx, tx, 1);
    // TODO: either get status (i.e. success/failure) from status register
    // or poll (transfer 0s) until you get a response from esp
}

// For Server: Obtains a list of clients currently connected to server
uint8_t *get_connected(void);
