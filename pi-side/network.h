#include "rpi.h"
#include "spi.h"
// #include "pi-esp.h"

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

// need to figure out how we represent to/from
// will need a way to compute checksums

uint8_t spi_chip = 0;  // just use chip 0 always

/* Prompt the esp to init itself as a station aka client in its setup
Note: might not use, might just flash client code to dedicated client esps */
uint8_t client_init(void);

/* Prompt the esp to init itself as an access point aka server in its setup
Note: might not use, might just flash server code to dedicated server esp */
uint8_t server_init(void);

uint8_t send_data_nbytes(uint8_t cmd, uint32_t to, uint32_t from, const void *bytes, uint32_t nbytes);

/* Receive data from esp by transferring 0's over SPI. Returns a buffer with the esp's
response or null if unsuccessful.*/
uint8_t receive_data_nbytes(void);

// For Client: Prompt client esp to connect to the server via Wifi.begin()
uint8_t connect_to_wifi(void);

// For Client: Returns whether or not this client pi's esp is connected to the server
uint8_t is_connected(void);

// For Server: Obtains a list of clients currently connected to server
uint8_t *get_connected(void);

// Write 32 byte buffer to ESP
void esp_put32(void *buf);

// Read 32 byte buffer from ESP
void esp_get32(void *buf);