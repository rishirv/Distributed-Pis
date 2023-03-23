#include "rpi.h"
#include "spi.h"
#include "sw-uart.h"
#include "pi-esp.h"
#include "fast-hash32.h"
#include "constants.h"
/* Commands encoded like so:
enum { 
    ESP_CLIENT_INIT         = 0b0001,
    ESP_SERVER_INIT         = 0b0010,
    ESP_SEND_DATA           = 0b0011,
    ESP_WIFI_CONNECT        = 0b0100,
    ESP_IS_CONNECTED        = 0b0101,
    ESP_GET_CONNECTED_LIST  = 0b0110,
    ESP_SERV_IP                 = 0b0111,
    ESP_ACK                 = 0b1000,
    ESP_NOACK               = 0b1001,
    ESP_GET_SERV_IP         =0b1010,
};*/

// As a client we only need to tell the esp to connect us to the server, if we cannot connect
// we return a -1. Otherwise we set the localFD and serverFD globals 
int client_init(sw_uart_t *u) {
    int stat = 0;
    for(int i = 0; i < 5; i++){
        stat = connect_to_wifi(u);
        if (stat != -1) break;
        delay_us(100000);
    }  
    
    if(stat == -1) return stat;

    send_cmd(u,0xf,0xf,ESP_GET_SERV_IP ,0,0);

    fd fds = get_fd(MAXFILES);
    while(!has_status(&fds));
    int statS = get_status(&fds);
    if (statS == ESP_FAIL) return -1;

    // success on both accounts we can continue
    localFD = stat;
    serverFD = statS;
    // server FD is obfuscated from user only return our FD 
    return stat;
}

/* Prompt the esp to init itself as an access point aka server in its setup
Note: might not use, might just flash server code to dedicated server esp */
int server_init(void) {
    send_cmd(u,ESP_SERVER_INIT,MAXFILES,MAXFILES,NULL,0);

    // now wait on the fd status to change, need to timeout on this
    fd fds = get_fd(MAXFILES);
    int i = 0;
    while(fds.status == NONE){
       fds =  get_fd(MAXFILES);
    }
    printk("got status correctly\n");
    
    uint8_t status = get_status(&fds);

    if(status != ESP_FAIL){
        // set both globals as we are the server
        localFD = status;
        serverFD = status;
        return status;
    }
    return -1;
}

/* Send command and data from pi to esp in 32 byte packets with the following form:
    Packet Headers (2 bytes) --> On every 32byte packet
    4 bits: To
    4 bits: From
    1 bit: isCmd
    5 bits: nbytes (in packet)
    2 bits: SBZ
    If isCmd: remaining bytes used for
    4 bytes --> Size of data being sent (2GB)
    1 bytes --> The actual Cmd (like connect to wifi or whatever)
    4 bytes --> Check sum of all the data

    Top 2 bytes of 32 byte packet:

    bits:    15 14 13 12 11 10 9   8   7 6 5 4 3 2 1 0
             | sbz |   nbytes   |isCmd| from  |  to   |
    byte:               31                 30

    Remaining bytes 29-0 are either data or the following if a command packet:
    
    byte:    29 28 27 26  25  24 23 22 21 20 ... 0
             | totalsize |cmd| checksum  |        |
*/
uint8_t send_cmd(sw_uart_t *u, uint8_t cmd, uint8_t to, uint8_t from, const void *bytes, uint32_t nbytes) {
 
    // first check to make sure we have a valid cmd - make sure to malloc it
    esp_cmnd_pckt_t *header = kmalloc(sizeof(esp_cmnd_pckt_t));
    // zero out everything then set fields
    memset(header, 0, 32);
    // Build the command packet header
    header->_sbz1 = 0;
    header->nbytes = CMD_NBYTES;
    header->isCmd = 1;
    header->esp_From = (from & 0xf);
    header->esp_To = (to & 0xf);
    // get checksum AFTER gathering all the data packets 
    header->cmnd = cmd;
    header->size = nbytes;

    //For sanity checking
    trace("CMD nybtes = %d\n", header->nbytes);
    trace("CMD isCmd = %d\n", header->isCmd);
    trace("CMD from = %d, to = %d\n", header->esp_From, header->esp_To);
    trace("CMD cmnd = %d\n", header->cmnd);
    trace("CMD size = %d\n", header->size);

    // If nybtes is not 0, i.e. we have data to send too...create/send data packets!    
    uint32_t bytes_left = nbytes;
    char *data = (char *)bytes;
    uint32_t npckts = (nbytes/DATA_NBYTES) + ((nbytes % DATA_NBYTES) > 0);
    
    esp_pckt_t *pckts = NULL;
    if (npckts) 
        pckts = kmalloc(sizeof(esp_pckt_t) * npckts); 
   
    // create all the data packets, store in an array, get cksum of that, THEN send!
    int curr_pkt = 0;
    while (bytes_left) {
        // Create data packet header
        uint8_t send_size = bytes_left >= 30 ? 30 : bytes_left % 30;
        esp_pckt_t pckt = {
            ._sbz = 0,
            .nbytes = send_size,
            .isCmd = 0,    // this is a data packet!
            .esp_From = (from & 0xf),
            .esp_To = (to & 0xf),
        };
        // Write a portion of data bytes to this packet
        memcpy(pckt.data, data, send_size);
        // store data packet in pckts array to send later
        pckts[curr_pkt] = pckt; 
        // update vars so we get next chunk of data
        bytes_left -= send_size;
        data += send_size;
        curr_pkt++;
    }

    // compute checksum over all the data packets and their headers 
    uint32_t checksum = fast_hash32(pckts,sizeof(esp_pckt_t) * PKT_NBYTES);
    header->cksum = checksum;

    // send the command packet!
    sw_uart_putPckt(u, header);

    // Now send all the data packets one by one
    for (int i = 0; i < npckts; i++)  {
        sw_uart_putPckt(u, &pckts[i]);
    }

    //TODO wait on a timeout for an ack back from the pi.
    // can use a get32B cmnd and idk a timeout of like 1/2 second to start
    //
    // - eventually this would look like us having a fd that we can read from and 
    // not having to busy wait here. 
    
    //TODO if ack recieved then return 1,
    //If No-ACK or timeout then resend from the top, probably keep retrans counter and 
    //eventually return -1 if the retrans maxes out. (this could be like a pi disconnected all of a sudden)
    return 1; 
}

int sendProgram(uint8_t to, const void * bytes, uint32_t nbytes){
    if (serverFD == 0) panic("attempting to send program without initializing server"); 

    // NOTE honestly retrans and acking might not work well here, theres room for error in the pattern, not to mention we dont have an easy way of numbering so we can throw away duplicates on the other side.. thats lower level than this. Likely just going off of a high timeout would be enough. Not fast of course

    // TODO consider sending some kind of ack req on repeat until it acks and lets us know its ready to recieve 
    send_cmd(u,ESP_SEND_DATA,to,serverFD,bytes,nbytes);

    // TODO wait for ack on a timeout .
    return 1;
}

void recProgram(){
    if (serverFD == 0) panic("attempting to recieve program without initializing client");
    
    //TODO send an ACK to server to let it know we are ready

    fd fds = get_fd(serverFD);
    
    while(!has_msg(&fds)) fds = get_fd(serverFD);
    
    //TODO do whatever we want to do with the program.. can return it if thats easiest
    
    //TODO send an ACK to let the serv know we got it and we are moving on
}


// For Client: Prompt client esp to connect to the server via Wifi.begin()
int connect_to_wifi(sw_uart_t *u) {
    // Call send_cmd with ESP_WIFI_CONNECT
    send_cmd(u,ESP_WIFI_CONNECT,0xf,0xf,NULL,0);

    // okay now we want to wait on our fds 
    fd fds = get_fd(MAXFILES);
    // wait till we see a status change which marks either a success or fail 
    while(fds.status == NONE) fds = get_fd(MAXFILES);
    
    // gets and clears status 
    int status = get_status(&fds);
    if (status == 0) return -1;
    
    // otherwise set the esp_id with the status (this is our ip for from addr);
     return status;
}

// For Server: Obtains a list of clients currently connected to server

uint8_t *get_connected(void) {
    char* buff = (char*)kmalloc(sizeof(char)*16);
    memset(buff,0,16);
    send_cmd(u,ESP_GET_CONNECTED_LIST,0xf,0xf,0,0);
    
    fd fds = get_fd(MAXFILES);
    int i = 0;
    while (1){
        while(!has_status(&fds) )fds = get_fd(MAXFILES);
        uint8_t stat = get_status(&fds);
        if(stat == ESP_DONE) break;
        buff[i] = stat;
        i++;
    }

     return buff;

}


