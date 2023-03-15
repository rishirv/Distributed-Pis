// Initialize connection with ESP
void client_esp_init(char pi_id);

// Broadcast to the server that this
// PI is not currently running a program
// Send the result of the previous computation.
void broadcast_avail(void *res);

// Read a program from the server into buf
// and return the number of bytes written.
// Blocks until a program is sent
uint32_t get_prog(void *buf);