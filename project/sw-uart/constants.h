
#include "sw-uart.h"
#ifndef __FDS_H__
    #include "fds.h"
#endif
#include "gpio.h"


sw_uart_t* u; 
fd *fileTable;


//sw_uart_mk_helper(21,23,9600,(700*1000*10000UL)/9600,(1000*1000)/9600);
