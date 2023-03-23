#include "rpi.h"
#include "constants.h"
#include "pi-esp.h"
#include "cycle-count.h"
#include "cycle-util.h"
// TODO should wrap most of our defines in these so we avoid redefinition hell
#ifndef __FDS_H__
    #include "fds.h"

#endif



//TODO  DONE move headers for basically all of below into a .h and rename this to .c
//TODO DONE see if we can move the struct fd into the header as well 
//Likely we are gonna get hosed if we move this to work inside a thread scheduler bc we can't make use
//of two #defined queues in the same space I think.. 
//the #define of constants can remain here in .c

// TODO DONE add method for initing the fd array: make it global by declaring it in the constants 
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
    fileTable = (fd*)kmalloc(sizeof(fd)*(MAXFILES+1));
    //TODO DONE loop thru init each one, malloc the initial message as well! 
    for(int i= 0; i < (MAXFILES +1); i++){
        msg_t* cur_msg = (msg_t*)kmalloc(sizeof(cur_msg));
        // init the values to be proper
        cur_msg->has_cmd = 0;
        cur_msg->cmd = 0;
        cur_msg->totPckts =0;
        cur_msg->curPckts=0;
        cur_msg->data=NULL; int k = 0;

        fileTable[i].cur_msg = cur_msg;
        //not sure if we need to init the Q 
        fileTable[i].status=NONE;
    }
}

fd get_fd(uint8_t fnum){
    if(fnum > MAXFILES) panic("INVALID FILE DESCRIPTOR NUMBER IN GET FD !");
    //basically we assume we have access to the global here 
    return fileTable[fnum];
}

// returns the msg pointer (malloced already), free is an issue here that will need be handled later
msg_t* get_msg(fd* fds){
    if (Q_empty(&fds->msg_q)) panic("attempting to pop empty q in get_msg");
    return Q_pop(&fds->msg_q);
}

int has_msg(fd* fds){
    //printk("hs msg curPckts: %d , Q_empty: %d\n", fds->cur_msg->curPckts, Q_empty(&fds->msg_q));
    return !Q_empty(&(fds->msg_q));
}

int has_status(fd* fds){
    return fds->status != NONE;
}
uint8_t get_status(fd* fds){
    uint8_t stat = fds->status;
    fds->status = NONE;
    return stat;
}

int add_msg(fd* fds){
   // if(fds->cur_msg->totPckts != fds->cur_msg->curPckts) panic("adding msg when pckt counts do not align");
    // allocate space for a new message 
    msg_t* nxtMsg = kmalloc(sizeof(msg_t));
    // init the values
    nxtMsg-> cmd = 0;
    nxtMsg->totPckts = 0;
    nxtMsg->curPckts = 0;
    nxtMsg->data = NULL;
    // push the message we have gathered up onto the queu
    //printk("Qempty: %d \n", Q_empty(&fds->msg_q));
    Q_push(&fds->msg_q,fds->cur_msg);
   // printk("Qempty: %d \n", Q_empty(&fds->msg_q));
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
    // TODO hardcoded fix later
    
    // read msg - all pulled in to ensure this is fast without timing issue: 
    char* buff = kmalloc(sizeof(esp_cmnd_pckt_t));
    int bytesRead = 0;
    // change as needed
    int timeout_usec = 5000;
    for (int i = 0; i< 32; i++, bytesRead++){
        int c = 0;
        // start a timeout timer
        uint32_t timer_st = timer_get_usec();
        // normally high will start reading once it goes low
        // If this is interrupt enabled then we only wnat to force the falling bit on subsequent bits, as we already know the first bit is falling.
        if( POLLING || i > 0){
            while(!gpio_read(u->rx) && (timer_get_usec() - timer_st) < timeout_usec);
        }
        while(gpio_read(u->rx) && (timer_get_usec() - timer_st) < timeout_usec);
        // if we fell through due to timeout then we return a -1
        if (timer_get_usec() - timer_st > timeout_usec) {
            c = -1;
            break;}

        int start = cycle_cnt_read();
        delay_ncycles(start,(u->cycle_per_bit) /2);
        start += u->cycle_per_bit /2;
    
        delay_ncycles(start,(u->cycle_per_bit) );
        start += u->cycle_per_bit;
    
        for(int i = 0; i <8; i++){
            c |= gpio_read(u->rx) << i;
            delay_ncycles(start,u->cycle_per_bit);
            start += u->cycle_per_bit;
        }
        if (c == -1) {
            bytesRead = i;
            break;
        }

        buff[i] = (char)c;
    }

    gpio_event_clear(RXPIN);
    // if we timeout then just return
    if (bytesRead < 32) {
        printk("failed: %d",bytesRead);
        return;
    }
    
   // printk("got message %s\n",buff);

    // ######################## WE HAVE A MESSAGE ##############################
    // cast to a pck_cmnd_strct 
    esp_cmnd_pckt_t* pckt = (esp_cmnd_pckt_t*)buff;
    
    fd* fds;
    // if we don't timeout then we have a msg
    // if both are 0xf then we place this into the special fd 
    if(pckt->esp_From == 0xf && pckt->esp_To == 0xf){
        //place into special fd;
        // so the 17th fd
        fds=fileTable+MAXFILES;
        fds->status = pckt->cmnd;
        return;
        //printk("special fds");
    }else{
        //otherwise take the from field
        fds= fileTable+pckt->esp_From;
    }

    msg_t* msg = fds->cur_msg;
    // now parse the packet: is is a cmnd? 
      if(pckt->isCmd){
         // printk("cmnd\n");
          //  If so is it an ACK/NOACK? : then change status line and return
        if(pckt->cmnd == ESP_ACK || pckt->cmnd == ESP_NOACK){
            fds->status = pckt->cmnd;
            printk("ack/noack\n");
            return;
        }
        // if we seen a command already then ignore the packet. 
        if(msg->has_cmd)  {
            msg->has_cmd = 0;
            msg->totPckts =0;
            msg->curPckts =0;
            msg->data = NULL;

            printk("seend msg already\n");
            return;
        }
        //okay parse the command as its good
        msg->has_cmd = 1;
        //should just be data packets
        msg->totPckts = (pckt->size /30)+(pckt->size % 30 > 0); 
        // if no data then we drop, doesnt make sense to compute an empty package 
        if(msg->totPckts == 0) {
            printk("err tot packets = 0\n");
            //return;
        }

        msg->curPckts = 0; // we havent seen a data packet yet
        // prepare the buffer for the message, not mallocing for headers: we strip those! 
        msg->data = kmalloc(30*msg->totPckts);
     //   printk("succesfull return from cmnd msg\n");
        return; 
    }else{
        //if we havent seen a cmnd pckt already then we are in error 
        if(!msg->has_cmd){
            msg->totPckts =0;
            msg->curPckts =0;
            msg->data =NULL;

            printk("err data packet but no cmnd packet seen\n");
            return;
        }

       // printk("message is data\n");
       //data packet
       esp_pckt_t* data_pckt = (esp_pckt_t*)pckt;
     //   printk("msg: [%s]\n",data_pckt->data);
       // nothing to really do here besides shove it onto the buffer! 
       memcpy(msg->data + (30*msg->curPckts),data_pckt->data,data_pckt->nbytes);
       msg->curPckts ++;
    }
    //  printk("im out yo\n\n");
    // Does cur pckts == tot=pckts? if so then run the checksum (todo) 
    // If it all checks out then place the mesg on the queue by calling add().  
     //printk("msg tot packets = %d",msg->totPckts);
      
     if (msg->curPckts == msg->totPckts){
       // TODO run chksm 
      // printk("adding data\n");
       add_msg(fds);
      }

}
