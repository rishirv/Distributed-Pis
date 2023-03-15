
COMMON_SRC += threads/thread-asm.S
COMMON_SRC += threads/rpi-thread.c 
COMMON_SRC += interrupts/interrupts-c.c 
COMMON_SRC += interrupts/timer-interrupt.c
COMMON_SRC += interrupts/interrupts-asm.S 

PROGS = $(wildcard *.c)
#PROGS := interrupts/timer.c

DIR:= $(CS140E_2023_PATH)/libpi
STAFF_OBJS := $(DIR)/staff-objs/kmalloc.o 

include $(CS140E_2023_PATH)/libpi/mk/Makefile.template
