#ifndef __VECTOR_BASE_SET_H__
#define __VECTOR_BASE_SET_H__
#include "libc/bit-support.h"
#include "asm-helpers.h"

/*
 * vector base address register:
 *   3-121 --- let's us control where the exception jump table is!
 *
 * defines: 
 *  - vector_base_set  
 *  - vector_base_get
 */

// return the current value vector base is set to.
static inline void *vector_base_get(void) {
    uint32_t* base;
    asm volatile("MRC p15, 0, %[val],c12,c0,0": [val] "=r"(base));
    
    return base;
}

// check that not null and alignment is good.
static inline int vector_base_chk(void *vector_base) {
    if( vector_base == NULL ) return 0;
    if (((uint32_t)vector_base & 0b11111) != 0) return 0;
    return 1;
}

// set vector base: must not have been set already.
static inline void vector_base_set(void *vec) {
    if(!vector_base_chk(vec))
        panic("illegal vector base %p\n", vec);

    void *v = vector_base_get();
    if(v) 
        panic("vector base register already set=%p\n", v);

    asm volatile("MCR p15,0,%[val],c12,c0,0"::  [val] "r" (vec));

    v = vector_base_get();
    if(v != vec)
        panic("set vector=%p, but have %p\n", vec, v);
}

// reset vector base and return old value: could have
// been previously set.
static inline void *
vector_base_reset(void *vec) {
    if(!vector_base_chk(vec))
        panic("illegal vector base %p\n", vec);

    void *old_vec = 0;
    old_vec = vector_base_get();
    asm volatile("MCR p15,0,%[val],c12,c0,0"::  [val] "r" (vec));
    assert(vector_base_get() == vec);
    return old_vec;
}
#endif
