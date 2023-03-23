#include "rpi.h"

/* Send data from pi to esp in 32 byte packets with the following form:
 * Packet Headers (2 bytes) --> On every 32byte packet
 * 4 bits: To
 * 4 bits: From
 * 1 bit: isCmd
 * 5 bits: nbytes (in packet)
 * 2 bits: SBZ
 */
typedef struct {
    uint8_t to : 4;
    uint8_t from : 4;
    uint8_t isCmd : 1;
    uint8_t packetBytes : 5;
    uint8_t sbz : 2;
} header_t;
_Static_assert(sizeof(header_t) == 2, "header size is wrong");

/* If isCmd: remaining bytes used for
 * 4 bytes --> Size of data being sent (2GB)
 * 1 bytes --> The actual Cmd (like connect to wifi or whatever)
 * 4 bytes --> Check sum of all the data
 * --> Only using 11 bytes for cmd so have space for other things if needed
 */
typedef struct {
    uint32_t dataBytes;
    uint8_t cmd;
    uint32_t cksumData;
} __attribute__((packed)) cmd_t;
_Static_assert(sizeof(cmd_t) == 9, "command size is wrong");

typedef struct {
    uint8_t data[30];
} data_t;
_Static_assert(sizeof(data_t) == 30, "data size is wrong");

typedef union {
    data_t data;
    cmd_t cmd;
} contents_t;
_Static_assert(sizeof(data_t) == 30, "contents size is wrong");

typedef struct {
    header_t header;
    contents_t contents;
} packet_t;
_Static_assert(sizeof(packet_t) == 32, "packet size is wrong");

#define CMD_BYTES (sizeof(cmd_t))
#define DATA_BYTES (sizeof(data_t))