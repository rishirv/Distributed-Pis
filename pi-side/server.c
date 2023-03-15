#include "server.h"
#include "network.h"

void *progs[];
void *results[];

int notmain() {
    server_esp_init();
    uint32_t pi_ids = all_pis();
}