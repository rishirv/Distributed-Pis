# need the makefile to give an error if this doesn't exist
PROGS := client.c

CFLAGS += -I..
CFLAGS += -I../../../server-client-sw_uart/lib

COMMON_SRC += ../../../server-client-sw_uart/lib/sw-uart.c
# uncomment if you want it to automatically run.
# RUN=1

# define this if you need to give the device for your pi
TTYUSB = /dev/cu.usbserial-10
BOOTLOADER = my-install

include $(CS140E_2023_PATH)/libpi/mk/Makefile.template
