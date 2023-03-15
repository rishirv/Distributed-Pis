#include "rpi.h"

// Initialize connection with ESP
void server_esp_init();

// Returns a bitfield with the ids of all
// PIs currently in the network.
uint32_t all_pis();

// Returns a bitfield with the ids of all
// PIs currently broadcasting their availability.
uint32_t free_pis(uint32_t pi_id);

// Send a program from the server to client pi_id
void send_pi_prog(uint8_t pi_id, uint32_t nbytes, void *code);

// ESP maintains return values from client broadcast_avail
// When the server discovers that a new pi is free, calls
// get_prog_res to get the result for that program
// Stores the result into the 32-byte buffer buf
void get_prog_res(uint32_t pi_id, void *buf);