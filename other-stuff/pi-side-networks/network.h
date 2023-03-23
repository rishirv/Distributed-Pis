#include "rpi.h"
#include "spi.h"
#include "sw-uart.h"
#include "network-types.h"
// #include "pi-esp.h"

// This PI's id - initialized on init
uint8_t my_id;
// IMPORTANT TODO: REMEMBER TO INIT
sw_uart_t *uart;

void write_packet(const packet_t *pkt);
packet_t read_packet();

packet_t make_command(header_t header, cmd_t cmd);
packet_t make_data(header_t header, data_t data);

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