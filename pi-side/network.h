#include "rpi.h"
#include "rpi.h"
#include "spi.h"

typedef struct {
    uint8_t to : 4;
    uint8_t from : 4;
    uint8_t isCmd : 1;
    uint8_t packetBytes : 5;
    uint8_t sbz : 2;
} header_t;

_Static_assert(sizeof(header_t) == 2, "header size is wrong");

typedef struct {
    uint32_t nbytes;
    uint8_t cmd;
    uint32_t cksumData;
} __attribute__((packed)) cmd_t;

_Static_assert(sizeof(cmd_t) == 9, "command size is wrong");

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
};*/

uint8_t spi_chip = 0;  // just use chip 0 always

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

/* Send data from pi to esp in 32 byte packets with the following form:
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
--> Only using 11 bytes for cmd so have space for other things if needed */
uint8_t send_data_nbytes(uint8_t cmd, uint32_t to, uint32_t from, const void *bytes, uint32_t nbytes) {
    // 1. send cmd along with header, will decompose building header later!
    uint8_t rx[32], tx[32];
    uint32_t hdr = 0;
    // convert uint32_t ip addresses to 4 bit encoding
    hdr |= (convert_ip(to) << 2);  // lower 2 bits SBZ
    hdr |= (convert_ip(from) << 6);
    // need to indicate this is a command
    hdr |= (1 << 10);
    // Leave nbytes as 0 since cmd packet holds no data

    // 2. send remaining nbytes of data
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

// Write 32 byte buffer to ESP
void esp_put32(void *buf);

// Read 32 byte buffer from ESP
void esp_get32(void *buf);