#include "rpi-asm.h"
MK_FN(check_pc_bounds)
    @assuming the pc is stored at address in r0
    push {r1-r12}
    ldr r1, [r0]
    ldr r2, =PUT32
    cmp r1, r2
    bllt oobpanic

    ldr r2, =__code_end__
    cmp r1,r2
    blge oobpanic
    pop {r1-r12}
    bx lr

