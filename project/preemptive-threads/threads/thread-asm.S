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

    push {r0-r12,lr}
    mrs r2,cpsr
    push {r2}

    str sp, [r0] @placing my updated sp at the address they gave
    
    mov  sp, r1 @moving to the new context

    pop {r0}
    msr cpsr, r0
    msr cpsr_c, #0b00010011

    pop {r0-r12,lr}
     
    @mov r0,sp
    @b rpi_print_regs
    bx lr

MK_FN(rpi_cswitch2)
@take in a pointer to the array we will push everything onto r0
@take a pointer to the array which we will pop from r1
    @MAKE SURE R0 IS THE END OF THE ARRAY 
    ADD r0, r0, #64
    STMDA r0!, {r0-r12,lr} 
    
    mov r5,r1
    mov r4,r0

    mrs r2, cpsr
    @got to make sure interrutps are back on for the save
    AND r2,r2,#0xFFFFFF3F
    mov r1,lr
    mov r0,sp
    STMDA r4!, {r0-r2}
    
    @okay everything is in the array hooray! 
    mov r0, r5
    @grab sp, old lr, new cpsr from array
    LDMIA r0!, {r1-r3}
    mov sp, r1
    mov lr, r2
    
    @okay we now have to move to IRQ and place r3 into spsr, we just cant keep old lr and jump to where the function stopped without it. 
    msr cpsr_c, #0b11010010
    @saves the spsr 
    msr spsr, r3
    @places our stack at the array
    mov sp,r0
    @get everything
    pop {r0-r12,lr}
    @get outta doge
    movs pc, lr 

    

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

MK_FN(rpi_halfswitch)
@assume we whave the sp in r0 and nothing needs pushed
@In IRQ mode 

msr cpsr_c, #0b11010011

mov sp, r0

@now the cpsr
pop {r0}
ORR r0, #0b11010011
msr cpsr, r0
@b rpi_print_regs
pop {r0-r12,lr}


@turn interrupts back on
msr cpsr_c, #0b00010011
@well that should be it we can enter the new thread now
bx lr

MK_FN(rpi_enter)
@we might not be in IRQ mode if we enter first so just throw into IRQ to begin

msr cpsr_c, #0b11010010 

@assume we have the saved regs pointer in r0
@in IRQ 
@saved regs is  sp, lr_old, spsr, r0-r12, lr_irq

@first snag the sp and old lr 
mov sp, r0
pop {r1,r2}

msr cpsr_c, #0b11010011 @super mode ints off
mov sp, r1
mov lr, r2

msr cpsr_c, #0b11010010 @IRQ ints off 

@now pop off the spsr
pop {r1}
msr spsr,r1

@now everything else 
pop {r0-r12,lr}

@should put us back where we were with interrupts on
movs pc,lr 
