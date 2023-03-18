#include "rpi.h"

typedef struct msg_t{
    uint8_t cmd;
    uint32_t npackets;
    void* data;
}msg_t;

#define E msg_t
#include "libc/Q/h"
typedef struct fd{
    msg* cur_msg;

}
