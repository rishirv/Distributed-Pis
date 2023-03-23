/*
 * implement these.  ordered in difficulty.
 */
#include "rpi-asm.h"

@ return the current stack pointer.
MK_FN(rpi_get_sp)
    asm_todo("implement this")
    bx lr

MK_FN(rpi_cswitch)
    @bl rpi_print_regs 

    push {r4-r11,lr}

    str sp, [r0] @placing my updated sp at the address they gave
    
    mov  sp, r1 @moving to the new context
    pop {r4-r11,lr}
    
    bx lr

@ [Make sure you can answer: why do we need to do this?]
@
@ use this to setup each thread for the first time.
@ setup the stack so that when cswitch runs it will:
@	- load address of <rpi_init_trampoline> into LR
@	- <code> into r1, 
@	- <arg> into r0
@ 
MK_FN(rpi_init_trampoline)
    @assuming r5 holds args
    @assuming r6 holds the code

    mov r0, r5
    mov r1, r6
    
    blx r1 @runs code 
    b rpi_exit
