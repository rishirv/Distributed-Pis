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
   t->saved_regs[SPSR_OFFSET] = ((cpsr_get() & ~0b11000000) & ~0b11111) | 0b10000 ;// turn interrupts on and user mode
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
    PUT32(arm_timer_IRQClear,1);
    cur_thread = scheduler_thread;
    trace("calling thread exit\n");
    rpi_enter(cur_thread->saved_regs); 

    panic("should not be here - pi_exit\n");
}

/****************************************************************************************/
/******************************* SCHEDULING BELOW ***************************************/
/****************************************************************************************/

//linked list for sleeping
typedef struct sleep_tcb{
    rpi_thread_t* tcb;
    uint32_t start_tm;
    uint32_t delay;
    uint32_t queue;
    uint32_t valid ;
    struct sleep_tcb *next;
} sleep_tcb_t;

static Q_t runq1,runq2,runq3,runq4;
#define NUM_QUEUES 5

uint32_t threadTimes[NUM_QUEUES] = {250, 250, 250, 250, 250};
Q_t* queues[NUM_QUEUES] = {&runq,&runq1,&runq2,&runq3,&runq4};
uint32_t threadRuns[NUM_QUEUES] = {0,0,0,0,0};
uint32_t cur_queue;
int sleeping = 0;

// time since last brought all processes to first queue
uint32_t resurfTime;
// how many usecs to wait before bringing all processes back to 1st queue
uint32_t resurfDelay = 1000000; 

sleep_tcb_t sleep_h = {.tcb = NULL, .start_tm = 0, .delay = 0, .next = NULL, .valid = 0};
sleep_tcb_t* sleep_head = &sleep_h;
sleep_tcb_t* sleep_tail = &sleep_h;

int wake(){
    uint32_t cur_time = timer_get_usec();
    sleep_tcb_t* cur = sleep_head->next;
    sleep_tcb_t* prev = sleep_head;
    int awoke = 0;
    while(cur != NULL){
  //      printk("cur delay to wake: %d \n",cur->delay);
        if(cur_time - cur->start_tm >= cur->delay){
  //          printk("waking\n");
            Q_push(queues[cur->queue],cur->tcb);

            if(sleep_tail ==cur) sleep_tail = prev;
            prev->next = cur->next;
            cur = cur->next;
            sleeping --;
            awoke++;
        }else{
            prev = prev->next;
            cur = cur->next;
        }
    }
 //   printk("awoke: %d\n",awoke);

    return awoke;
}

// transfers all elements in the queues onto the top queue. 
// This can be costly on overhead so must be careful about how many times it gets called
void resurf(){
    //printk("running resurf\n");
    for (int i = 1; i < NUM_QUEUES; i ++){
        while(!Q_empty(queues[i])){
            Q_push(queues[0],Q_pop(queues[i]));
        }
    }
    resurfTime = timer_get_usec();
}

/*void threadSched(){
    trace("running scheduler\n");
    if(Q_empty(&runq)){
        goto end;
    }
    int_init();
    timer_interrupt_init(250);
    while(!Q_empty(&runq)){
     //   printk("in sched\n");
        cur_thread = Q_pop(&runq);
        PUT32(arm_timer_IRQClear,1);
        // todo run a hash check on the cur thread bfr cswitch
  //      printk("calling c_switch\n");
       rpi_cswitch2(scheduler_thread->saved_regs,cur_thread->saved_regs);
 //      printk("after  c\n");
    }
    end:
        th_trace("done with all threads, returning\n");
        clean_reboot();
}*/


void threadSched(){
    trace("running scheduler\n");
    if(Q_empty(&runq)){
        goto end;
    }
    int_init();
    timer_interrupt_init(250);
    while(1){
        for(int i = 0; i < NUM_QUEUES; i++){
            wake();

            // resurfaces into the top queue ever resurfDelay usecs
           if(timer_get_usec() - resurfTime > resurfDelay) resurf();

            cur_queue = i;
            if(Q_empty(queues[cur_queue])) continue;

            cur_thread = Q_pop(queues[cur_queue]);
            threadRuns[cur_queue] += 1;

            
            PUT32(arm_timer_IRQClear,1);
            rpi_cswitch2(scheduler_thread->saved_regs,cur_thread->saved_regs);
            i = -1;
        }
        // if nothing is blocking then we are truly done
        if(sleeping == 0 && Q_empty(&runq)) {
            printk("breaking no more sleeping\n");
            break;
        };
        //otherwise we block until a thread is ready to run 
         while(!wake() && Q_empty(&runq));
    }
    end:
    for(int i =0; i< NUM_QUEUES; i++){
        printk("Queue %d ran %d \n", i, threadRuns[i]);
    }
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
    resurfTime = timer_get_usec();

    rpi_enter(scheduler_thread->saved_regs);
    printk("We should not be here!! \n");     
    int_init();
    timer_interrupt_init(250);
    system_enable_interrupts();
    PUT32(arm_timer_IRQClear,1);

       
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

// called on yield, does not demote thread in runq
void save_context_yield(uint32_t* sp){
    // copying the stack in our IRQ to the stack in our thread block
    memcpy(cur_thread->saved_regs,sp,sizeof(uint32_t)*17);

   // printk("switching context yield\n");
    Q_append(queues[0],cur_thread);
    PUT32(arm_timer_IRQClear,1);
    cur_thread = scheduler_thread;
    rpi_enter(cur_thread->saved_regs); 
}

// called only from preemption, demotes the thread in the runq
void save_context(uint32_t* sp){
    // copying the stack in our IRQ to the stack in our thread block
    memcpy(cur_thread->saved_regs,sp,sizeof(uint32_t)*17);
    
    int t_queue = cur_queue;
    t_queue += (t_queue == NUM_QUEUES-1) ? 0:1;
 //   printk("cur queue is %d \n",t_queue);
 //   printk("switching context non yield\n");
    Q_append(queues[t_queue],cur_thread);

    PUT32(arm_timer_IRQClear,1);
    cur_thread = scheduler_thread;
    rpi_enter(cur_thread->saved_regs); 
}


void rpi_sleep(uint32_t delay,uint32_t * sp){
    memcpy(cur_thread->saved_regs,sp,sizeof(uint32_t)*17);
    
   // printk("sleep was called from %d\n",cur_thread->tid);
    
    sleeping ++;
    sleep_tcb_t* cur = kmalloc(sizeof(sleep_tcb_t));
    cur->tcb = cur_thread;
    cur->queue = cur_queue;
    cur->start_tm = timer_get_usec();
    cur->delay = delay;
    cur->next = NULL;

    sleep_tail->next = cur;
    sleep_tail = sleep_tail->next;


    rpi_thread_t old = *cur_thread;
    cur_thread = scheduler_thread;
    rpi_cswitch2(old.saved_regs,scheduler_thread->saved_regs);
}

void rpi_simple_trace(uint32_t* arg1, uint32_t* arg2){
    trace("\n");
    trace("First arg reg is %x\n",arg1);
    trace("second arg reg is %x\n",arg2);
}
