#include "network.h"

void write_packet(const packet_t *pkt) {
    uint8_t *data = (uint8_t *) pkt;
    for (int i = 0; i < sizeof(*pkt); i++) {
        sw_uart_put8(uart, data[i]);
    }
}

packet_t read_packet() {
    packet_t pkt;
    uint8_t *data = (uint8_t *) &pkt;
    for (int i = 0; i < sizeof(pkt); i++) {
        data[i] = sw_uart_get8(uart);
    }
    return pkt;
}

packet_t make_command(header_t header, cmd_t cmd) {
    contents_t contents = {.cmd = cmd};
    packet_t pkt = {.header = header, .contents = contents};
    return pkt;
}

packet_t make_data(header_t header, data_t data) {
    contents_t contents = {.data = data};
    packet_t pkt = {.header = header, .contents = contents};
    return pkt;
}

// /* Prompt the esp to init itself as a station aka client in its setup
// Note: might not use, might just flash client code to dedicated client esps */
// uint8_t client_init(void) {
//     uint8_t rx[32], tx[32];
//     tx[0] = ESP_CLIENT_INIT;
//     // not sure about the clock divider but this is what Dawson used
//     // see pin_init from lab 16
//     spi_t s = spi_n_init(spi_chip, 20);
//     spi_n_tranfer(s, rx, tx, 1);
//     /* TODO: either get status (i.e. success/failure) from status register, see lines 48-50:
//     https://github.com/esp8266/Arduino/blob/master/libraries/SPISlave/examples/SPISlave_Test/SPISlave_Test.ino
//     OR poll (transfer 0s) until you get a response from esp */
// }

// /* Prompt the esp to init itself as an access point aka server in its setup
// Note: might not use, might just flash server code to dedicated server esp */
// uint8_t server_init(void) {
//     uint8_t rx[32], tx[32];
//     tx[0] = ESP_SERVER_INIT;
//     // not sure about the clock divider but this is what Dawson used
//     // see pin_init from lab 16
//     spi_t s = spi_n_init(spi_chip, 20);
//     spi_n_tranfer(s, rx, tx, 1);
//     // TODO: either get status (i.e. success/failure) from status register
//     // or poll (transfer 0s) until you get a response from esp
// }

// /* Send data from pi to esp in 32 byte packets with the following form:
// Packet Headers (2 bytes) --> On every 32byte packet
// 4 bits: To
// 4 bits: From
// 1 bit: isCmd
// 5 bits: nbytes (in packet)
// 2 bits: SBZ
// If isCmd: remaining bytes used for
// 4 bytes --> Size of data being sent (2GB)
// 1 bytes --> The actual Cmd (like connect to wifi or whatever)
// 4 bytes --> Check sum of all the data
// --> Only using 11 bytes for cmd so have space for other things if needed */
// uint8_t send_data_nbytes(uint8_t cmd, uint32_t to, uint32_t from, const void *bytes, uint32_t nbytes) {
//     // 1. send cmd along with header, will decompose building header later!
//     uint8_t rx[32], tx[32];
//     uint32_t hdr = 0;
//     // convert uint32_t ip addresses to 4 bit encoding
//     hdr |= (convert_ip(to) << 2);  // lower 2 bits SBZ
//     hdr |= (convert_ip(from) << 6);
//     // need to indicate this is a command
//     hdr |= (1 << 10);
//     // Leave nbytes as 0 since cmd packet holds no data

//     // 2. send remaining nbytes of data
// }

// // For Client: Prompt client esp to connect to the server via Wifi.begin()
// uint8_t connect_to_wifi(void) {
//     uint8_t rx[32], tx[32];
//     tx[0] = ESP_WIFI_CONNECT;
//     // not sure about the clock divider but this is what Dawson used
//     // see pin_init from lab 16
//     spi_t s = spi_n_init(spi_chip, 20);
//     spi_n_tranfer(s, rx, tx, 1);
//     // TODO: either get status (i.e. success/failure) from status register
//     // or poll (transfer 0s) until you get a response from esp
// }

// // For Client: Returns whether or not this client pi's esp is connected to the server
// uint8_t is_connected(void) {
//     uint8_t rx[32], tx[32];
//     tx[0] = ESP_WIFI_CONNECT;
//     // not sure about the clock divider but this is what Dawson used
//     // see pin_init from lab 16
//     spi_t s = spi_n_init(spi_chip, 20);
//     spi_n_tranfer(s, rx, tx, 1);
//     // TODO: either get status (i.e. success/failure) from status register
//     // or poll (transfer 0s) until you get a response from esp
// }