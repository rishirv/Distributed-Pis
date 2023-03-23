# the tests in decreasing order of difficulty.
PROGS := ./client.c

CFLAGS_EXTRA = -Iexternal-code -I../lib

# a list of all of your object files.
COMMON_SRC := $(wildcard ../lib/*.[Sc])
COMMON_SRC += $(wildcard ./external-code/*.[Sc])

# TTYUSB = 
BOOTLOADER = my-install
RUN = 0

EXCLUDE ?= grep -v simple_boot
GREP_STR := 'HASH:\|ERROR:\|PANIC:\|PASS:\|TEST:'
include $(CS140E_2023_PATH)/libpi/mk/Makefile.template-v2

clean::
	rm -f ../lib/*~
	make -C external-code clean
