PROG_SRC = shell.c
SRC = simple-boot.c put-get.c
CFLAGS += -I..

include $(CS140E_2023_PATH)/libunix/mk/Makefile.unix

run: ./shell
	make -C ../server
	cp ../server/server.bin server.bin
	@./shell