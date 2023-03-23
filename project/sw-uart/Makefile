# first make sure hello.c works.

# "make checkoff" should run everything and check
# easiest output: do this first with the sw-uart-veneer.c
#PROGS = $(wildcard ./tests/output-test-0-hello.c)

# then this
#PROGS = $(wildcard ./tests/output-test*.c)
# do this first to develop
PROGS = test_pi_datapkts.c
PROGS = hello.c
PROGS = recieveServ.c
#PROGS = sendClient.c
#PROGS = 1-getc-test.c
#PROGS = test_pi_serverinit.c
PROGS = tests/connList/connList.c
#PROGS = tests/pingpong1.c
PROGS = tests/sndRec_test1/recieveServ.c
#PROGS = tests/connList/connList.c

PROGS = tests/clientRec.c
PROGS = tests/sendServ.c


#PROGS = tests/sndRec_test1/recieveServ.c
#once your sw-uart works: add sw-uart-veneer.c
 #COMMON_SRC += sw-uart-veneer.c
COMMON_SRC += interrupts-c.c
COMMON_SRC += interrupts-asm.S
COMMON_SRC += sw-uart.c
COMMON_SRC += pi-esp.c
COMMON_SRC += fds.c 
COMMON_SRC += gpio-int.c

# can use the staff code to see that things work.
# STAFF_OBJS := staff-sw-uart.o

# uncomment if you want it to automatically run.
 RUN=1

BOOTLOADER = my-install

EXCLUDE = grep -v simple_boot
include $(CS140E_PROJ_PATH)/libpi/mk/Makefile.template-v2

checkoff:
	@echo "you should have copied sw-uart.o into your libpi/src directory";
	@echo "and added it to your makefile";
	make -C ./ COMMON_SRC="sw-uart-veneer.c" PROGS="$(wildcard tests/output-test*.c)" check

clean::
	make -C tests clean
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
