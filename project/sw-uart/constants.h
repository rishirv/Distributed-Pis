
#include "sw-uart.h"
#ifndef __FDS_H__
    #include "fds.h"
#endif
#include "gpio.h"

#define POLLING 0
#define TXPIN 23
#define RXPIN 21


uint8_t localFD;
uint8_t serverFD;
sw_uart_t* u; 
fd *fileTable;


//sw_uart_mk_helper(21,23,9600,(700*1000*10000UL)/9600,(1000*1000)/9600);
