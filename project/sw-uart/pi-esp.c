#include "pi-esp.h"
#include "sw-uart.h"
uint8_t send_cmd(uint8_t cmd, uint8_t to, uint8_t from, const void *data, uint32_t nbytes){

    // first check to make sure we have a valid cmd - make sure to malloc it
    
    // TODO formulate and send the cmnd pckt

    // TODO If nbytes is larger than 0 - malloc and formulate the general data pckt hdr
    // then loop until nbytes = 0, sending 32 bytes or nbytes%32 of data
    // I think easiest to just sub bytes send from nbytes until it hits 0. 
    // adjusting the size of hdr pckt accordingly. 
    // since this is a serial line we dont worry about ordering. 
    
    
    //TODO wait on a timeout for an ack back from the pi.
    // can use a get32B cmnd and idk a timeout of like 1/2 second to start
    //
    // - eventually this would look like us having a fd that we can read from and 
    // not having to busy wait here. 
    
    //TODO if ack recieved then return 1,
    //If No-ACK or timeout then resend from the top, probably keep retrans counter and 
    //eventually return -1 if the retrans maxes out. (this could be like a pi disconnected all of a sudden)

    return 1; 
}
