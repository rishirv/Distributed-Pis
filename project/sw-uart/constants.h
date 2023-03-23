
#include "sw-uart.h"
#ifndef __FDS_H__
    #include "fds.h"
#endif
#include "gpio.h"

#define TRACE 0
#define POLLING 0

#define TXPIN 23
#define RXPIN 21


uint8_t localFD;
uint8_t serverFD;

sw_uart_t* u; 
fd *fileTable;


