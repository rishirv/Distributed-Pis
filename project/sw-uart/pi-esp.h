#include "rpi.h"
#include "sw-uart.h"

// need an enum for the instruction codes

// need to figure out how we represent to/from

// will need a way to compute checksums

#define HEADER_SIZE 2
#define CMD_NBYTES 9 // we don't include 2-byte header in size of data in a packet
#define DATA_NBYTES 30 // data packets can hold a max of 30 bytes of data after header
#define PKT_NBYTES 32

#define ESP_FAIL 0b0000

#define SELF 0xf // use for to and from when sending a command to your own pi
#define MAX_NCLIENTS 15
#define ESP_DONE 0b11111
enum { 
    ESP_CLIENT_INIT         = 0b0001,
    ESP_SERVER_INIT         = 0b0010,
    ESP_SEND_DATA           = 0b0011,
    ESP_WIFI_CONNECT        = 0b0100,
    ESP_IS_CONNECTED        = 0b0101,
    ESP_GET_CONNECTED_LIST  = 0b0110,
    ESP_GET_SERV_IP         = 0b0111,
    ESP_ACK                 = 0b1000,
    ESP_NOACK               = 0b1001,
};



typedef struct esp_pckt {
    uint16_t
        _sbz:2,
        nbytes:5,
        isCmd:1,
        esp_From:4,
        esp_To:4;
    uint8_t data[30];
} esp_pckt_t;

typedef struct esp_cmnd_pckt {
    uint16_t
        _sbz1:2,
        nbytes:5,
        isCmd:1,
        esp_From:4,
        esp_To:4;
    uint32_t cksum;
    uint8_t cmnd;
    uint32_t size;
    uint8_t _sbz[13];
} esp_cmnd_pckt_t;


void system_init(int server);

/* Prompt the esp to init itself as a station aka client in its setup
Note: might not use, might just flash client code to dedicated client esps */
int client_init(sw_uart_t *u);

int sendProgram(uint8_t to, const void *bytes, uint32_t nbytes);

/* Prompt the esp to init itself as an access point aka server in its setup
Note: might not use, might just flash server code to dedicated server esp */
int server_init(void);

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
uint8_t send_cmd(sw_uart_t* u, uint8_t cmd, uint8_t to, uint8_t from, const void *data, uint32_t nbytes);

/* Receive data from esp by transferring 0's over SPI. Returns a buffer with the esp's
response or null if unsuccessful.*/
uint8_t receive_data_nbytes(sw_uart_t *u);

// For Client: Prompt client esp to connect to the server via Wifi.begin()
int connect_to_wifi(sw_uart_t *u);

//NOTE not implemented/used
// For Client: Returns whether or not this client pi's esp is connected to the server
uint8_t is_connected(sw_uart_t *u);

// For Server: Obtains a list of clients currently connected to server
int get_connected(uint8_t* buff);

// TODO: Also add a method for client pi's to periodically check that they are connected to
// the server and if not, reconnect!
