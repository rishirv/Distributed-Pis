#include "rpi.h"
#include "your-prototypes.h"

void oobpanic(uint32_t* lr){
    panic("PC is out of bounds at %x",lr);
}
