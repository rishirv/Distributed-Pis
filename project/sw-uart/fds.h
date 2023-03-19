#include "rpi.h"

#define __FDS_H__

#define NONE 0b0000
#define MAXFILES 17 // 16 for 4 bits worth + 1 for system reserved
// malloc the msg and put the pointer on the queue
typedef struct msg_t{
    uint8_t has_cmd;
    uint8_t cmd;
    uint32_t totPckts;
    uint32_t curPckts;
    char* data;
    struct msg_t* next;
}msg_t;

#define E msg_t
#include "libc/Q.h"
 
// holds all the msgs
typedef struct fd{
    msg_t* cur_msg;
    Q_t msg_q;
    uint8_t status;
}fd;

void init_fileTable();

fd get_fd(uint8_t fnum);



// returns the msg pointer (malloced already), free is an issue here that will need be handled later
msg_t* get_msg(fd* fds);

int has_msg(fd* fds);

uint8_t get_status(fd* fds);

int add_msg(fd* fds);


// has u from constants
// note: it would be nice to No-Ack back if there was an issue, however that means our 
// handler will be constrained to the speed of our baud and a 32byte send. This isnt gonna fly in a multithread world so we will ignore and let timeouts handle things on the other end. 

// we just clear current message on a return;
void recieveMsgHandler();

