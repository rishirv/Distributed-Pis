#ifndef __ARMV6_DEBUG_IMPL_H__
#define __ARMV6_DEBUG_IMPL_H__
#include "armv6-debug.h"

// all your code should go here.  implementation of the debug interface.

// example of how to define get and set for status registers
//coproc_mk(status, p14, 0, c0, c1, 0)


//coproc_mk(dscr,cp14,0,c0,c1,0)
//coproc_mk_set(dscr,cp14,0,c0,c1,0)

coproc_mk_get(wcr0,p14,0,c0,c0,7)
coproc_mk_set(wcr0,p14,0,c0,c0,7)

// 13-26
coproc_mk_get(wvr0,p14,0,c0,c0,6)
coproc_mk_set(wvr0,p14,0,c0,c0,6)

coproc_mk_get(ifsr,p15,0,c5,c0,1)

coproc_mk_get(dscr,p14,0,c0,c1,0)
coproc_mk_set(dscr,p14,0,c0,c1,0)

coproc_mk_get(dfsr,p15,0,c5,c0,0)

coproc_mk_get(wfar,p14,0,c0,c6,0)
// double check the second c0
coproc_mk(bcr0,p14,0,c0,c0,5)
coproc_mk(bvr0,p14,0,c0,c0,4)
coproc_mk(ifar,p15,0,c6,c0,2)
coproc_mk(far,p15,0,c6,c0,0)
// you'll need to define these and a bunch of other routines.

// return 1 if enabled, 0 otherwise.  
//    - we wind up reading the status register a bunch:
//      could return its value instead of 1 (since is 
//      non-zero).
static inline int cp14_is_enabled(void) {
    uint32_t v = cp14_dscr_get();
    return bit_is_on(v,15) && bit_is_off(v,14);
    
}

// enable debug coprocessor 
static inline void cp14_enable(void) {
    // if it's already enabled, just return?
    if(cp14_is_enabled())
        panic("already enabled\n");
    
    // for the core to take a debug exception, monitor debug mode has to be both 
    // selected and enabled --- bit 14 clear and bit 15 set.
    uint32_t v = cp14_dscr_get();
    v = bit_set(v,15);
    v = bit_clr(v,14);
    cp14_dscr_set(v);

    assert(cp14_is_enabled());
}

// disable debug coprocessor
static inline void cp14_disable(void) {
    if(!cp14_is_enabled())
        return;
    
    uint32_t v = cp14_dscr_get();
    v = bit_clr(v,15);
    cp14_dscr_set(v);

    assert(!cp14_is_enabled());
}


static inline int cp14_bcr0_is_enabled(void) {
   uint32_t v = cp14_bcr0_get();
   return bit_is_on(v,0);
}

static inline void cp14_bcr0_enable(void) {
    uint32_t v = cp14_bcr0_get();
    v = bit_set(v,0);
    cp14_bcr0_set(v); 
    assert(cp14_bcr0_is_enabled());
}
static inline void cp14_bcr0_disable(void) {
    uint32_t v = cp14_bcr0_get();
    v = bit_clr(v,0);
    cp14_bcr0_set(v);
    assert(!cp14_bcr0_is_enabled());
}

// was this a brkpt fault?
static inline int was_brkpt_fault(void) {
    // use IFSR and then DSCR
    // check IFSR first
    uint32_t v1 = cp15_ifsr_get();
    if (bit_is_on(v1,10) || bits_get(v1,0,3) != 2){
        return 0;
    };
    // check DSCR
    uint32_t v2 = cp14_dscr_get();
    if(bits_get(v2,2,5) != 1){
        return 0;
    }
    return 1;
}

// was watchpoint debug fault caused by a load?
static inline int datafault_from_ld(void) {
    return bit_isset(cp15_dfsr_get(), 11) == 0;
}
// ...  by a store?
static inline int datafault_from_st(void) {
    return !datafault_from_ld();
}


// 13-33: tabl 13-23
static inline int was_watchpt_fault(void) {
    // use DFSR then DSCR
    uint32_t v1 = cp15_dfsr_get();

    if (bit_is_on(v1,10) || bits_get(v1,0,3) != 2){
        return 0;
    };
    // check DSCR
    uint32_t v2 = cp14_dscr_get();
    if(bits_get(v2,2,5) != 2){
        return 0;
    }
    return 1;
}

static inline int cp14_wcr0_is_enabled(void) {
    uint32_t v = cp14_wcr0_get();
    return bit_is_on(v,0);
}
static inline void cp14_wcr0_enable(void) {
    uint32_t v = cp14_wcr0_get();
    v = bit_set(v,0);
    cp14_wcr0_set(v);
    assert(cp14_wcr0_is_enabled());
}
static inline void cp14_wcr0_disable(void) {
    uint32_t v = cp14_wcr0_get();
    v = bit_clr(v,0);
    cp14_wcr0_set(v);
    assert(! cp14_wcr0_is_enabled());
}


// Get watchpoint fault using WFAR
static inline uint32_t watchpt_fault_pc(void) {
    uint32_t v = cp14_wfar_get();
    return v - 0x8;
}
    
#endif
