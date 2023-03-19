#include "rpi.h"
#include "pi-esp.h"
#include "constants.h"

#define NONE 0b0000
#define MAXFILES 17 // 16 for 4 bits worth + 1 for system reserved
// malloc the msg and put the pointer on the queue
typedef struct msg_t{
    uint8_t has_cmd;
    uint8_t cmd;
    uint32_t totPckts;
    uint32_t curPckts;
    void* data;
}msg_t;

#define E msg_t
#include "libc/Q/h"
 
// holds all the msgs
typedef struct fd{
    msg* cur_msg;
    Q_t msg_q;
    uint8_t status;
}fd


//TODO move headers for basically all of below into a .h and rename this to .c
//TODO see if we can move the struct fd into the header as well 
//Likely we are gonna get hosed if we move this to work inside a thread scheduler bc we can't make use
//of two #defined queues in the same space I think.. 
//the #define of constants can remain here in .c

// TODO add method for initing the fd array: make it global by declaring it in the constants 
//  the init method will just go through malloc some fds for each one and set the initial state correctly
//  NOTE we will need to init the queue and be sure to test it well! 
//
//
//TODO okay after all that then we can test what we have to make sure it compiles okay, maybe we can just // hard call the get msg while sending a bunch of stuff periodically from the esp, then chekcing to see if we recieved the message. We can turn up the timeout and we can add some prints for failure, 
//then we can send a fixed number of packets and loop it to recieve said packets then check if the queue is there and works. if we get here we have hit a big milestone! 

// TODO now we shove in the interrupts: we want to go look at the threading to see how this is done, 
//  really just need to init the interrupt vector somewhere: Ordering likely matters so we will need to make sure we init the filetable and constants before turning on interrupts. 
//  make sure we branch to the interrupt vector in this file. 
//
//
//TODO  once we make it here, we should test the interrupts by sending a message or two and waiting for the fds to have something. this should be pretty clear if this works or not. 
//
//TODO now we turn our attention to the arduino, we see whats up and we got from there. 
//likely build it out real hard... then we finish up the methods on our side... and then I think we are basically there where we wanted to be. 
//
void init_fileTable(){
    fd fileTable[MAXFILES] = (fd*) kmalloc(sizeof(fd)*MAXFILES);
    //TODO loop thru init each one, malloc the initial message as well! 

    //TODO make sure to set the global filtable from constants!! 
}

fd* get_fd(uint8_t fnum){
    if(fnum >= MAXFILES-1) panic("INVALID FILE DESCRIPTOR NUMBER IN GET FD !");
    //TODO get the file descriptor pointer from the table and return it!
}



// returns the msg pointer (malloced already), free is an issue here that will need be handled later
msg_t* get_msg(fd* fds){
    if (Q_empty(&fds->msg_q)) panic("attempting to pop empty q in get_msg");
    return Q_pop(&fds->msg_q);
}

int has_msg(fd* fds){
    return Q_empty(&fds->msg_q);
}

uint8_t get_status(fd* fds){
    uint8_t stat = fds->status;
    fds->status = NONE;
    return stat;
}

int add_msg(fd* fds){
    if(totPckts != curPckts) panic("adding msg when pckt counts do not align");
    // allocate space for a new message 
    msg* nxtMsg = kmalloc(sizeof(msg));
    // init the values
    nxtMsg-> cmd = 0;
    nxtMsg->totPckts = 0;
    nxtMsg->curPckts = 0;
    nxtMsg->data = NULL;
    // push the message we have gathered up onto the queu
    Q_push(&fds->msg_q,cur_msg);
    // now point cur message to the new empty message we made
    fds->cur_msg = nxtMsg;
    //return 1 for success
   return 1; 
}


// has u from constants
// note: it would be nice to No-Ack back if there was an issue, however that means our 
// handler will be constrained to the speed of our baud and a 32byte send. This isnt gonna fly in a multithread world so we will ignore and let timeouts handle things on the other end. 

// we just clear current message on a return;
void recieveMsgHandler(){
    // read msg
    char* buff = kmalloc(sizeof(esp_cmnd_pckt));

    // timeout at 1, see if this is too slow. seems slow to me for an interrupt but not much to be done rn. 
    int succ = sw_uart_get32B(u,1,buff);
    // if we timeout then just return
    if (succ < 33) return;
    // cast to a pck_cmnd_strct 
    esp_cmnd_pckt* pckt = (esp_cmnd_pckt*)buff;
    
    fd* fds;
    // if we don't timeout then we have a msg
    // if both are 0xf then we place this into the special fd 
    if(pckt->esp_From == 0xf && pckt->esp_To == 0xf){
        //place into special fd;
        //TODO 
    }else{
        //otherwise take the from field
        // get the fd (just index into the global fd table for now)
        // TODO
    }
    

    msg_t* msg = fds->cur_msg;
    // now parse the packet: is is a cmnd? 
      if(pckt->isCmd){
          //  If so is it an ACK/NOACK? : then change status line and return
        if(pckt->cmnd == ESP_ACK || pckt->cmnd == ESP_NOACK){
            fds->status = pckt->cmnd;
            return;
        }
        // if we seen a command already then ignore the packet. 
        if(msg->has_cmd)  {
            msg->has_cmd = 0;
            msg->totPckts =0;
            msg->curPckts =0;
            msg->data = nullptr;
            return;
        }
        //okay parse the command as its good
        msg->has_cmd = 1;
        msg->totPckts = 0// TODO do some math here.. cpy from orig should just be data packets 
        // TODO drop it if totPckts = 0 ... doesn't make sense to send 0 packets! besides errs in the malloc
        msg->curPckts = 0; // we havent seen a data packet yet
        // prepare the buffer for the message, not mallocing for headers: we strip those! 
        msg->data = kmalloc(sizeof(char) pckt->size);
        // okay we got it all we can return;
        return; 
    }else{
       //data packet
       esp_pckt_t* data_pckt = (esp_pckt_t*)pckt;

       // nothing to really do here besides shove it onto the buffer! 
       memcpy(msg->data[30*msg->curPckts],data_pckt->data,30);
       curPckts ++;
    }
    
    // Does cur pckts == tot=pckts? if so then run the checksum (todo) 
    // If it all checks out then place the mesg on the queue by calling add().  
      if (msg->curPckts == msg->totPckts){
       // TODO run chksm 
       add_msg(fds);
      }

}
