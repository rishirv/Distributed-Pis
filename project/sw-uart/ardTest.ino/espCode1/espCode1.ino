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
  uint8_t sbz[13];
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
  esp_cmnd_pckt* cmnd_pckt = NULL;
}pi_buff;

// eventually this should be an array of these structs.
pi_buff* from_pi = (pi_buff*)malloc(sizeof(pi_buff));

//TODO eventually take in which pi we are writing the message to, leave be for now. 
void write_msg_pi(){
  esp_cmnd_pckt cmd_pckt;
  memset(&cmd_pckt,0,32);
  cmd_pckt._sbz1 = 0;
  cmd_pckt.nbytes = 9;
  cmd_pckt.isCmd = 1;
  cmd_pckt.esp_From = 0xa;
  cmd_pckt.esp_To = 0xa;
  cmd_pckt.cmnd = 0xf;
  cmd_pckt.size = 35;

  esp_pckt data;
  data._sbz1 =0;
  data.nbytes = 6;
  data.esp_From = 0xa;
  data.esp_To = 0xa;
  data.isCmd = 0;
  memset(data.data,0,30);
  char st[30] = "hello world this a twenty six";
  for(int i = 0; i < 30; i++){
    data.data[i] = st[i];
  } 

  esp_cmnd_pckt* cmd_pp= &cmd_pckt;
  char* cmd_p = (char*)cmd_pp;
  for(int i = 0; i <32;i++){
    mySerial.print(cmd_p[i]);
  }
  /*
  for(int i = 0; i < 32; i++){
    mySerial.print(cmd_p[i]);
  }*/
  int k = 0;
  while(k < 100000){
    yield();
    k++;
  }

  esp_pckt* data_pp = &data;
  char* data_p = (char*)data_pp;
  for (int i =0; i< 32; i++){
    mySerial.print(data_p[i]);
  }

  char tt[6] = "yargh";
  for(int i = 0; i < 6; i++){
    data.data[i] = tt[i];
  } 
 k = 0;
  while(k < 100000){
    yield();
    k++;
  }
   for (int i =0; i< 32; i++){
    mySerial.print(data_p[i]);
  }
  /*
  for(int i = 0; i < 32; i++){
    mySerial.write(buff[i]);
  }*/
  //mySerial.print(cmd_pckt);
  //mySerial.print(data);
}

// error in reading pi packet 
void read_err_pi(){
 Serial.println("READ ERROR PI");
 //reset our buffer state, making sure to free the buffer

// okay not sure why but the free is causing issues i.e pehaps buff is the wrong value besides null
 if  (from_pi-> buff!= NULL){
   free(from_pi->buff);
   from_pi->buff = NULL;
 }

// resetting, dont memset thatll be bad times
 from_pi->cmnd = 0;
 from_pi->numPckts = 0;
 from_pi->curPckts = 0;
 from_pi->runRdy = 0;

 // drain the read buffer in case we are out of sync
 while(mySerial.available()){
   mySerial.read();
   yield();
 }
 //TODO  write a no-ack
}

// this will eventually extend parse between multiple To's, for now just assumes one buffer.
void parsePacket(char* packet){
 
  if(from_pi->runRdy){
    Serial.println("need to run current command before parsing new one");
    return read_err_pi();
  }

  esp_cmnd_pckt* pckt = (esp_cmnd_pckt*)packet;

  if (pckt->nbytes == 0) {
    Serial.println("failed nbytes is 0");
    read_err_pi();
    return;
    }

  // great its a cmd (probably do more checking on other fields if time/space permit)
  if(pckt->isCmd){
      // error if we have already seen a cmnd, or there isnt a command in the packet
      if(from_pi->cmnd > 0 || pckt->cmnd == 0) {
        if (from_pi->cmnd >0) Serial.println("already seen cmnd");
        if (pckt->cmnd == 0) Serial.println("cmnd empty on pckt");
       // Serial.println("error cmnd already seen in pckt");
        return read_err_pi();
      }
      // allocate buffer space for the full message, assign size and command to the struct
      from_pi->buff = (esp_pckt*)malloc(sizeof(esp_pckt) * ((pckt->size /30)+(pckt->size % 30 > 0)));
      from_pi->numPckts = ((pckt->size /30)+(pckt->size % 30 > 0));
      from_pi->cmnd = pckt->cmnd;
      *from_pi->cmnd_pckt = *pckt;
  }else{
    // its data, check for errors 
    if (from_pi->cmnd == 0 || from_pi->buff == NULL) return read_err_pi();
    
    // We actually want to include all the header packets bc it will be needed on the other side as well. 
    esp_pckt * data_pckt = (esp_pckt*)pckt;

    from_pi->buff[from_pi->curPckts] = *data_pckt;
    from_pi->curPckts ++;
  }
  // check if we have filled the command data size
  if(from_pi->numPckts == from_pi->curPckts){
    //TODO check checksum - this might require a bit bc we arent including the headersin the chksum 
  // but I just shoved the headers in the data buffer here. 
    from_pi->runRdy = 1;
  }
  // TODO send ack back 
}

void parseNreadPckt(){
    char* buff = (char*)malloc(sizeof(char) * 32);
    for (int i = 0; i < 32; i++){
        buff[i] = mySerial.read();
    }
   // write_msg_pi(buff,32);
    parsePacket(buff);
    free(buff);
}

void runCmnd(){
  // printing everything out 
    Serial.println("Hooray running a commmand, but not really cause I dont have it together");
    Serial.printf("Num packets: %d \n",from_pi->numPckts);
    Serial.printf("Cmnd: %x \n",from_pi->cmnd);
    Serial.printf("To: %x , FROM: %x \n", from_pi->cmnd_pckt->esp_To,from_pi->cmnd_pckt->esp_From);
    Serial.printf("cksum: %x\n",from_pi->cmnd_pckt->cksum);
    Serial.printf("size: %x\n ", from_pi->cmnd_pckt->size);
    Serial.print("Printing sbz zone: [");
    for (int i = 0; i < 13; i++){
      Serial.printf("%x",from_pi->cmnd_pckt->sbz[i]);
    }
    Serial.println("]");
    
    Serial.printf("Printing data buff headers\n");
    for(int i = 0; i < from_pi->numPckts; i++){
      esp_pckt data_pckt = from_pi->buff[i];
      Serial.printf("Data pckt TO: %x, FROM: %x \n",data_pckt.esp_To,data_pckt.esp_From);
      Serial.printf("Data pckt size in bytes: %d \n", data_pckt.nbytes);
      char* data = (char*) data_pckt.data;
      Serial.print('[');
      for(int i = 0; i < 30; i++){
        Serial.printf("%c",data[i]);
      }
      Serial.println(']');
    }

    read_err_pi();
}

void setup() {
  Serial.begin(115200);
  Serial.println("\nstarted serial!\n");
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);

  mySerial.begin(9600);

  Serial.printf("size of cmnd: %d \n", sizeof(esp_cmnd_pckt));
  // setup from Pi buffer
   from_pi->runRdy= 0;
  from_pi->buff = NULL;
  from_pi-> numPckts = 0;
  from_pi->curPckts = 0;
  from_pi-> cmnd = 0;
  // probs dont need to malloc, was just for sanity earlier.
  from_pi->cmnd_pckt = (esp_cmnd_pckt*)malloc(sizeof(esp_cmnd_pckt));
}

int i = 0;
void loop() {
  //Serial.println(mySerial.available());
  if(from_pi->runRdy) return runCmnd();
  if(mySerial.available() > 31) return parseNreadPckt();
  if(i%1000000 == 0){
    Serial.println("writing message");
  write_msg_pi();
  }
  i++;
}
