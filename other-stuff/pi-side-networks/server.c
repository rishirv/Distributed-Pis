#include "server.h"
#include "network.h"
#include "constants.h"
#include "boot-crc32.h"

uint8_t min(uint8_t x, uint8_t y) {
    return x > y ? y : x;
}

void send_pi_prog(uint8_t pi_id, uint32_t nbytes, void *code) {
    uint32_t crc = crc32(code, nbytes);
    header_t header = {.from = my_id, .to = pi_id, .isCmd = 1, .packetBytes = CMD_BYTES};
    cmd_t cmd = {.dataBytes = nbytes, .cmd = ESP_SEND_PROG, .cksumData = crc};
    packet_t pkt = make_command(header, cmd);
    write_packet(&pkt);

    header.isCmd = 0;
    header.packetBytes = DATA_BYTES;

    while (nbytes > 0) {
        uint8_t packet_bytes = min(nbytes, DATA_BYTES);
        header.packetBytes = packet_bytes;
        data_t data;
        memcpy(data.data, code, packet_bytes);

        pkt = make_data(header, data);
        write_packet(&pkt);

        nbytes -= packet_bytes;
        code = (uint8_t *) code + packet_bytes;
    }
}

void notmain() {
}