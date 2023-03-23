// engler, cs140e: starter code for trivial threads package.
#include "rpi.h"
#include "rpi-thread.h"
#include "timer-interrupt.h"

// tracing code.  set <trace_p>=0 to stop tracing
enum { trace_p = 1};
#define th_trace(args...) do {                          \
    if(trace_p) {                                       \
        trace(args);                                   \
    }                                                   \
} while(0)

/***********************************************************************
 * datastructures used by the thread code.
 *
 * you don't have to modify this.
 */

#define E rpi_thread_t
#include "libc/Q.h"

// currently only have a single run queue and a free queue.
// the run queue is FIFO.
static Q_t runq, freeq;
static rpi_thread_t *cur_thread;        // current running thread.
static rpi_thread_t *scheduler_thread;  // first scheduler thread.
void rpi_print_regs(uint32_t *sp);
// monotonically increasing thread id: won't wrap before reboot :)
static unsigned tid = 1;

/***********************************************************************
 * simplistic pool of thread blocks: used to make alloc/free faster (plus,
 * our kmalloc doesn't have free (other than reboot).
 *
 * you don't have to modify this.
 */

// total number of thread blocks we have allocated.
static unsigned nalloced = 0;

// keep a cache of freed thread blocks.  call kmalloc if run out.
static rpi_thread_t *th_alloc(void) {
    rpi_thread_t *t = Q_pop(&freeq);

    if(!t) {
        t = kmalloc_aligned(sizeof *t, 8);
        nalloced++;
    }
#   define is_aligned(_p,_n) (((unsigned)(_p))%(_n) == 0)
    demand(is_aligned(&t->stack[0],8), stack must be 8-byte aligned!);
    t->tid = tid++;
    return t;
}

static void th_free(rpi_thread_t *th) {
    // push on the front in case helps with caching.
    Q_push(&freeq, th);
}
static inline uint32_t cpsr_get(void){
    uint32_t cpsr;
    asm volatile("mrs %0, cpsr" : "=r" (cpsr));
    return cpsr;
}

/***********************************************************************
 * implement the code below.
 */

// stack offsets we expect.
enum {
    SP_OFFSET = 0,
    LR_OLD_OFFSET,
    SPSR_OFFSET,
    R0_OFFSET,
    R1_OFFSET,
    R2_OFFSET,
    R3_OFFSET,
    R4_OFFSET,
    R5_OFFSET,
    R6_OFFSET,
    R7_OFFSET,
    R8_OFFSET,
    R9_OFFSET,
    R10_OFFSET,
    R11_OFFSET,
    R12_OFFSET,
    R14_OFFSET = 16,
    LR_OFFSET = 16
};

// return pointer to the current thread.  
rpi_thread_t *rpi_cur_thread(void) {
    return cur_thread;
}

// create a new thread.
rpi_thread_t *rpi_fork(void (*code)(void *arg), void *arg) {
    rpi_thread_t *t = th_alloc();
  

    // write this so that it calls code,arg.
    void rpi_init_trampoline(void);

    
    t->saved_sp = t->stack + (THREAD_MAXSTACK-17);
    t->saved_regs[SP_OFFSET] = (uint32_t)t->saved_sp;
   t->saved_regs[SPSR_OFFSET] = cpsr_get() & ~0b11000000; // turn interrupts on
    t->saved_regs[R5_OFFSET] = (uint32_t)arg;
    t->saved_regs[R6_OFFSET] = (uint32_t)code;
   t->saved_regs[LR_OFFSET] = (uint32_t)rpi_init_trampoline;
 
    t->fn = code;
    t->arg = arg;

    th_trace("rpi_fork: tid=%d, code=[%p], arg=[%x], saved_sp=[%p]\n",
            t->tid, code, arg, t->saved_sp);
    Q_append(&runq, t);
    return t;
}

// same as fork but we dont append to the runQ bc thats a bad bad idea! 
rpi_thread_t * rpi_forkSched(void(*code)(void*arg),void *arg){
    
    rpi_thread_t *t = th_alloc();
    // write this so that it calls code,arg.
    void rpi_init_trampoline(void);
    
    t->saved_sp = t->stack + (THREAD_MAXSTACK-17);
    t->saved_regs[SP_OFFSET] = (uint32_t)t->saved_sp;
   t->saved_regs[SPSR_OFFSET] = cpsr_get();
    t->saved_regs[R5_OFFSET] = (uint32_t)arg;
    t->saved_regs[R6_OFFSET] = (uint32_t)code;
   t->saved_regs[LR_OFFSET] = (uint32_t)rpi_init_trampoline;
 
    t->fn = code;
    t->arg = arg;

    th_trace("rpi_fork: tid=%d, code=[%p], arg=[%x], saved_sp=[%p]\n",
            t->tid, code, arg, t->saved_sp);
    return t;
}
// exit current thread.
//   - if no more threads, switch to the scheduler.
//   - otherwise context switch to the new thread.
//     make sure to set cur_thread correctly!
void rpi_exit(int exitcode) {
    system_disable_interrupts();
    trace("Hit exit \n");
    // when you switch back to the scheduler thread:
    rpi_thread_t* old = cur_thread;
    
    Q_push(&freeq,cur_thread);
    if(Q_empty(&runq)){
        cur_thread = scheduler_thread;
         th_trace("done running threads, back to scheduler\n");
        rpi_cswitch2(old->saved_regs,scheduler_thread->saved_regs);
    }else {
          //cur_thread  = Q_pop(&runq);
          cur_thread = scheduler_thread;
        trace("jumping from %d back into next thread %d\n",old->tid,cur_thread->tid);
          rpi_cswitch2(old->saved_regs, scheduler_thread->saved_regs);
    }
}

// yield the current thread.
//   - if the runq is empty, return.
//   - otherwise: 
//      * add the current thread to the back 
//        of the runq (Q_append)
//      * context switch to the new thread.
//        make sure to set cur_thread correctly!
void rpi_yield(void) {
    system_disable_interrupts();
    if(Q_empty(&runq)){
        return;
    }
     Q_append(&runq,cur_thread);
      rpi_thread_t* old = cur_thread; 
        rpi_thread_t* new = Q_pop(&runq);
        cur_thread = new; 
    // if you switch, uncomment
     PUT32(arm_timer_IRQClear,1);
      //th_trace("switching from tid=%d to tid=%d\n", old->tid, new->tid);
        rpi_cswitch2(old->saved_regs,new->saved_regs);
}


void threadSched(){
    trace("running scheduler\n");
    if(Q_empty(&runq)){
        goto end;
    }
    int_init();
    timer_interrupt_init(250);
    while(!Q_empty(&runq)){
        cur_thread = Q_pop(&runq);
        PUT32(arm_timer_IRQClear,1);
        // todo run a hash check on the cur thread bfr cswitch
        rpi_cswitch2(scheduler_thread->saved_regs,cur_thread->saved_regs);
    }
    end:
        th_trace("done with all threads, returning\n");
        clean_reboot();
}
/*
 * starts the thread system.  
 * note: our caller is not a thread!  so you have to 
 * create a fake thread (assign it to scheduler_thread)
 * so that context switching works correctly.   your code
 * should work even if the runq is empty.
 */
void rpi_thread_start(void) {
    th_trace("starting threads!\n");

    // no other runnable thread: return.
    if(Q_empty(&runq))
        goto end;

    // setup scheduler thread block.
    if(!scheduler_thread){
        int empty = -1;
        scheduler_thread = rpi_forkSched(threadSched,&empty);
        cur_thread = scheduler_thread;
    }
    rpi_enter(scheduler_thread->saved_regs);
    printk("We should not be here!! \n");     
    int_init();
    timer_interrupt_init(250);
    system_enable_interrupts();
    PUT32(arm_timer_IRQClear,1);

  //  todo("implement the rest of rpi_thread_start");
       
        cur_thread = Q_pop(&runq);
        th_trace("switchign into new thread\n");
       
        // cant quite bc we arent in IRQ mode... 
        rpi_enter(cur_thread->saved_regs);

        th_trace("should not be here\n");

end:
    th_trace("done with all threads, returning\n");
}



// helper routine: can call from assembly with r0=sp and it
// will print the stack out.  it then exits.
// call this if you can't figure out what is going on in your
// assembly.
void rpi_print_regs(uint32_t *sp) {
    printk("made it to regs\n");
    // use this to check that your offsets are correct.
    printk("cur-thread=%d\n", cur_thread->tid);
    printk("sp=%p\n", sp);

    // stack pointer better be between these.
    printk("stack=%p\n", &cur_thread->stack[THREAD_MAXSTACK]);
//    assert(sp < &cur_thread->stack[THREAD_MAXSTACK]);
  //  assert(sp >= &cur_thread->stack[0]);
    for(unsigned i = 0; i < 16; i++) {
        unsigned r = i == 13 ? 14 : i;
        printk("sp[%d]=r%d=%x\n", i, r, sp[i]);
    }
    //clean_reboot();
}

void save_context(uint32_t* sp){

    // copying the stack in our IRQ to the stack in our thread block
    // memcpy to a new array
    memcpy(cur_thread->saved_regs,sp,sizeof(uint32_t)*17);

    //todo recalc hash and save it 

   // rpi_print_regs(cur_thread->saved_sp);
    Q_append(&runq,cur_thread);
    
        if(Q_empty(&runq)){
        printk("exiting");
        return;
    }
    //Now would be a good time to clear the timer interrupt!! 
    //
    //dev_barrier();
    PUT32(arm_timer_IRQClear,1);
    //dev_barrier();

    // we dont care about the old thread since we just saved it so just get the new thread

    //cur_thread = Q_pop(&runq);
    cur_thread = scheduler_thread;
    //rpi_print_regs(cur_thread->saved_sp);
    //trace("about to call halfswitch \n");
    rpi_enter(cur_thread->saved_regs); 
}

void rpi_simple_trace(uint32_t* arg1, uint32_t* arg2){
    trace("\n");
    trace("First arg reg is %x\n",arg1);
    trace("second arg reg is %x\n",arg2);
}
