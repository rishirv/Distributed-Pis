#include <SoftwareSerial.h>

#define rxPin 14
#define txPin 12

SoftwareSerial mySerial = SoftwareSerial(rxPin, txPin);

typedef struct esp_cmnd_pckt{
  uint16_t
    _sbz1:2,
    nbytes:5,
    isCmd:1,
    esp_From:4,
    esp_To:4;
  uint32_t cksum;
  uint8_t cmnd;
  uint32_t size;
  uint8_t sbz[21];
} esp_cmnd_pckt;

typedef struct esp_pckt{
  uint16_t
    _sbz1:2,
    nbytes:5,
    isCmd:1,
    esp_From:4,
    esp_To:4;
  uint8_t data[30];
} esp_pckt;

// buffer for reading a cmnd. eventually turn into some kind of array of empty buffer structs
typedef struct pi_buff{
  int runRdy= 0;
  char* buff = NULL;
  int dataSize = 0;
  int curData = 0;
  uint8_t cmnd = 0;
  uint32_t cksum;
}pi_buff;

// eventually this should be an array of these structs.
pi_buff* from_pi;

// error in reading pi packet 
void read_err_pi(){
 Serial.println("READ ERROR PI");
 //reset our buffer state, making sure to free the buffer
 if  (from_pi-> buff!= NULL){
   free(from_pi->buff);
 }
 memset(from_pi,0,sizeof(pi_buff));

 // drain the read buffer in case we are out of sync
 while(mySerial.available()){
   mySerial.read();
   yield();
 }
 // write a no-ack
}

// this will eventually extend parse between multiple To's, for now just assumes one buffer.
void parsePacket(uint8_t* packet){

  if(from_pi->runRdy){
    Serial.println("need to run current command before parsing new one");
    return read_err_pi();
  }
  esp_cmnd_pckt* pckt = (esp_cmnd_pckt*)packet;

  if (pckt -> sbz > 0 || pckt->nbytes == 0) return read_err_pi();
  // great its a cmd (probably do more checking on other fields if time/space permit)
  if(pckt->isCmd){
      // error if we have already seen a cmnd, or there isnt a command in the packet
      if(from_pi->cmnd > 0 || pckt->cmnd == 0) return read_err_pi();

      // allocate buffer space for the packet, assign size and command to the struct
      from_pi->buff = (char *)malloc(sizeof(char)* pckt->size);
      from_pi->dataSize = pckt->size;
      from_pi->cmnd = pckt->cmnd;
      from_pi->cksum = pckt->cksum;
  }else{
    // its data, check for errors 
    if (from_pi->cmnd == 0 || from_pi->buff == NULL) return read_err_pi();

    // TODO okay strip header and add to buffer. not sure probably need a for loop here. would be nice to do without
    // so much branching but its what we got. 
  }
  // check if we have filled the command data size
  if(from_pi->dataSize == from_pi->curData){
    from_pi->runRdy = 1;
  }

  // TODO send ack back 
}

void readPckt(){
  //TODO loop read pckt into some kind of buffer, 
  
  // CALL parse Packet passing along the buffer

  // free the buffer. 
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("\nstarted serial!\n");
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);

  mySerial.begin(9600);

}

void loop() {
  
  if (mySerial.available() == 32){
    Serial.println("got 32 bytes");
    //Serial.println("seen something at leas");
    char buff[32];
    for (int i =0; i < 32; i++){
      buff[i] = mySerial.read();
    }
    esp_cmnd_pckt * cmnd = (esp_cmnd_pckt*) buff;

    if (cmnd->isCmd) {
      Serial.println("isCmnd checks out");
    }
    if (cmnd->esp_To == 0b1111){
      Serial.println("Esp to checks out");
    }
  }
}
