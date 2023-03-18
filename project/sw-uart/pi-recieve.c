#include "sw-uart.h"
#include "rpi.h"

// Eventually we will move this code into its rightfull place as an interrupt handler. For now we will jsut write it in a c file
//
// TODO setup a header file for this when needed
//
// TODO setup necessary structs somewhere.. 
//
// TODO make a file descriptor object somewhere... 


// Thoughts: Okay lets think about who is going to be using these fds .. well they are going to be accessible by anyone anywhere.. where the hell do we put something like that... 
// Seems to me like we can define the structs in rpi.h since thats used just about anywhere. 
// okay next we need to figure out where we hold the memory for said structs.. I think ultimately it will be in the thread scheduler .. so literalyl running in the kernel... that means a read should trap? ... okay yeah a read should definitely trap into a system call but damn thats a lot of work ... so no trapping into sytstem 
//
// What do we do insted?? well heres what we do for now: 
//
//
// 1. I will write up the structs in rpi.h
// 2. I will write a fds init function that will for now basically malloc all this shit and return a pointer into memory of a malloced array of fds.  
// 3.the init function sets a global value for the array addr
//
// 4. I will write a get(fds) function that basically returns a (const?) reference to the file descriptor using the global table. 
// 5. Why? Well that way when we got to add this in for realsies into the sched, we have a getter method we can replace just like that easy peasy instead of hunting down every access and changing it. It will force us to write correctly first time. 
// 6. okay then I can write out the logic. 
