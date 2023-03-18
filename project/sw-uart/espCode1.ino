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
  esp_pckt* buff = NULL;
  int numPckts = 0;
  int curPckts = 0;
  uint8_t cmnd = 0;
  esp_cmnd_pckt cmnd_pckt;
}pi_buff;

// eventually this should be an array of these structs.
pi_buff* from_pi = (pi_buff*)malloc(sizeof(pi_buff));

// error in reading pi packet 
void read_err_pi(){
 Serial.println("READ ERROR PI");
 //reset our buffer state, making sure to free the buffer

// okay not sure why but the free is causing issues i.e pehaps buff is the wrong value besides null
 /*if  (from_pi-> buff!= NULL){
   free(from_pi->buff);
 }*/
 memset(from_pi,0,sizeof(pi_buff));

 // drain the read buffer in case we are out of sync
 while(mySerial.available()){
   mySerial.read();
   yield();
 }
 // write a no-ack
}

// this will eventually extend parse between multiple To's, for now just assumes one buffer.
void parsePacket(char* packet){
  Serial.println("Parsing packet in parse");
  if(from_pi->runRdy){
    Serial.println("need to run current command before parsing new one");
    return read_err_pi();
  }
  esp_cmnd_pckt* pckt = (esp_cmnd_pckt*)packet;
//pckt -> sbz > 0 || 
  if (pckt->nbytes == 0) {
    Serial.println("failed  first one sbz is not 0 or nbytes is 0");
    read_err_pi();
    return;
    }
  // great its a cmd (probably do more checking on other fields if time/space permit)
  if(pckt->isCmd){
      Serial.println("Packet is a command");
      // error if we have already seen a cmnd, or there isnt a command in the packet
      if(from_pi->cmnd > 0 || pckt->cmnd == 0) {
        if (from_pi->cmnd >0) Serial.println("already seen cmnd");
        if (pckt->cmnd == 0) Serial.println("cmnd empty on pckt");
       // Serial.println("error cmnd already seen in pckt");
        return read_err_pi();
      }

      // allocate buffer space for the packet, assign size and command to the struct
      from_pi->buff = (esp_pckt*)malloc(sizeof(esp_pckt) * ((pckt->size /30)+(pckt->size % 30 > 0)));
      from_pi->numPckts = ((pckt->size /30)+(pckt->size % 30 > 0));
      from_pi->cmnd = pckt->cmnd;
      from_pi->cmnd_pckt = *pckt;
  }else{
    Serial.println("Packet is data");
    // its data, check for errors 
    if (from_pi->cmnd == 0 || from_pi->buff == NULL) return read_err_pi();
    
    // We actually want to include all the header packets bc it will be needed on the other side as well. 
    esp_pckt * data_pckt = (esp_pckt*)pckt;

    Serial.println("about to print data");
    for (int i = 0; i < 30;i++){
      Serial.printf("%c",data_pckt->data[i]);
    }
    
    Serial.println();
   // Serial.println(data_pckt->data)
    from_pi->buff[from_pi->curPckts] = *data_pckt;
    from_pi->curPckts ++;
  }
  // check if we have filled the command data size
  // change to packets
  if(from_pi->numPckts == from_pi->curPckts){
    //TODO check checksum - this might require a bit bc we arent including the headersin the chksum 
  // but I just shoved the headers in the data buffer here. 
    from_pi->runRdy = 1;
  }
  // TODO send ack back 
}

void parseNreadPckt(){
  Serial.println("parsing packet in ParseNread");
  //TODO loop read pckt into some kind of buffer, 
    char* buff = (char*)malloc(sizeof(char) * 32);

    for (int i = 0; i < 32; i++){
        buff[i] = mySerial.read();
        Serial.printf("%c",buff[i]);
    }

    Serial.println();
    parsePacket(buff);
  // CALL parse Packet passing along the buffer
    free(buff);
  // free the buffer. 
}

void runCmnd(){
    Serial.println("Hooray running a commmand, but not really cause I dont have it together");
    Serial.println("ignore the following err, its just being reused to clear the buff");
    read_err_pi();
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("\nstarted serial!\n");
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);

  mySerial.begin(9600);

  // setup from Pi buffer
   from_pi->runRdy= 0;
  from_pi->buff = NULL;
  from_pi-> numPckts = 0;
  from_pi->curPckts = 0;
  from_pi-> cmnd = 0;
}

void loop() {

  if(from_pi->runRdy) return runCmnd();
  if(mySerial.available() > 31) return parseNreadPckt();
  
 
  /*
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
  }*/
}
