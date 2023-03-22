#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <SoftwareSerial.h>

#define rxPin 14
#define txPin 12
#define PISPEC 0xf
#define BYTESpMSG 30
#define ESP_FAIL 0b0000

enum { 
    INIT                    = 0b0010,
    ESP_SEND_DATA           = 0b0011,
    ESP_WIFI_CONNECT        = 0b0100,
    ESP_IS_CONNECTED        = 0b0101,
    ESP_GET_CONNECTED_LIST  = 0b0110,
    ESP_NOP                 = 0b0111,
    ESP_ACK                 = 0b1000,
    ESP_NOACK               = 0b1001,
};

const char* ssid = "poop";
const char* password = "password";

SoftwareSerial mySerial = SoftwareSerial(rxPin, txPin);
WiFiClient client; 

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

//############################# CLIENT SPECIFIC##########################
int serverPort = 1001;
IPAddress serverIP =0;

// TODO we should have a server ip and port # this could be hardcoded or otherwise
void client_wifi_cnct(){
   if(WiFi.status() != WL_CONNECTED){
     int i = 0;
     // try to connect 10 times
     for(; i < 10; i++){
       if(WiFi.begin(ssid,password)== WL_CONNECTED) break;
     }
     // if we couldnt connect then fail 
     if (i == 10){
       Serial.println("didnt find a network connection");
       write_msg_pi(ESP_FAIL);
       return;
     }
   }
   serverIP = WiFi.gatewayIP();
  // have network connection, try to connect a tcp server
   if (!serverIP || !client.connect(serverIP,serverPort)){
     Serial.println("whoops failed to conncect tcp");
     write_msg_pi(ESP_FAIL);
   }
   else{
     Serial.printf("local ip is %x", WiFi.localIP()[3]);
     // success we can write our (the client) ip to the pi
     write_msg_pi((uint8_t)(WiFi.localIP()[3] & 0b1111));
   }
}
//#####################################################################

//############################# SERVER SPECIFIC##########################

// all the client objects and switch case code for that lol
// a whole array of pi_buffs we can index into for sending

void get_connected_list(){
  // first send an ack to the pi to let it know we are working on it
  write_msg_pi(ESP_ACK);

  // create a buffer
  // next we will need to go through each object in the long list of 16 objects
  // it will have to be repeated for each wifi client as we cant iterate over them basically
  // for each pair of clients: if its valid get its lwr 4 ip and left shift it by 4 (otherwise its 0) then or it with 
  // the other client and then add that uint8_t to the array at proper index. 

  // then we take that buffer of 8 uint8_ts and we send it to the pi manually 
  // then from there the pi will pick it up and cast it to a struct and decipher it. 

  // honestly gona wait a hot minute for this one! kinda the last thing I care about rn
  
}

//#####################################################################
// eventually this should be an array of these structs.
pi_buff* from_pi = (pi_buff*)malloc(sizeof(pi_buff));

// relays info from the wifi clients down the pipe to the pi
void relay_to_pi(WiFiClient client){
  while(client.available() % 32){
    for(int i =0; i < 32; i++){
      mySerial.write(client.read());
    }
    // avoid resets if lots of data is being sent
    yield();
  }
};

//Call this if we see an ack/nock as we dont buffer messages so we just pass this down the line
// TODO take in which pi_buffer we should be looking
void send_nack(){
  send_msg();
}

// TODO: take in which pi_buffer we should be looking at, i.e the esp_to 
void send_msg(){
  //TODO get which client we need to send to using cmnd_pckt->esp_To
  //okay pretend we have client
  char* cmnd_pckt = (char*)from_pi->cmnd_pckt;
  for(int i = 0; i < 32;i++){
    client.write(cmnd_pckt[i]);
  }
  char* data = (char*)from_pi->buff;
  for(int i = 0; i < from_pi->numPckts;i++){
    for (int k = 0; k < 32;k++){
      client.write(data[k+(i*k)]);
    }
  }
};

// used to send msg originating from esp to pi, sends 4 bits of data or an ACK/NOACK. 
void write_msg_pi(uint8_t data){
  Serial.printf("data send to pi %x\n",data);
  // we know its always going have a specific cmnd packet
  esp_cmnd_pckt cmd_pckt;
  memset(&cmd_pckt,0,32);
  cmd_pckt.nbytes = 9;
  cmd_pckt.cmnd = data;
  cmd_pckt.isCmd = 1;
  cmd_pckt.esp_From = PISPEC;
  cmd_pckt.esp_To = PISPEC;
  //memset(cmd_pckt.data,msg,nbytes);

  //cast and send the cmd packet.
  esp_cmnd_pckt* cmd_pp= &cmd_pckt;
  char* cmd_p = (char*)cmd_pp;
  for(int i = 0; i <32;i++){
    mySerial.print(cmd_p[i]);
  }
}


// error in reading pi packet 
void clean(){
// Serial.println("READ ERROR PI");
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
    return clean();
  }

  esp_cmnd_pckt* pckt = (esp_cmnd_pckt*)packet;

  if (pckt->nbytes == 0) {
    Serial.println("failed nbytes is 0");
    clean();
    return;
    }

  // great its a cmd (probably do more checking on other fields if time/space permit)
  if(pckt->isCmd){
      // error if we have already seen a cmnd, or there isnt a command in the packet
      if(from_pi->cmnd > 0 || pckt->cmnd == 0) {
        if (from_pi->cmnd >0) Serial.println("already seen cmnd");
        if (pckt->cmnd == 0) Serial.println("cmnd empty on pckt");
       // Serial.println("error cmnd already seen in pckt");
        return clean();
      }
      // allocate buffer space for the full message, assign size and command to the struct
      from_pi->buff = (esp_pckt*)malloc(sizeof(esp_pckt) * ((pckt->size /30)+(pckt->size % 30 > 0)));
      from_pi->numPckts = ((pckt->size /30)+(pckt->size % 30 > 0));
      from_pi->cmnd = pckt->cmnd;
      *from_pi->cmnd_pckt = *pckt;
  }else{
    // its data, check for errors 
    if (from_pi->cmnd == 0 || from_pi->buff == NULL) return clean();
    
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
    parsePacket(buff);
    free(buff);
}

void sanityPrint(){
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

    clean();
}

void runCmnd(){
  IPAddress ip;
  uint8_t lsb;
  switch(from_pi->cmnd) {
    case INIT:
      Serial.println("Got ESP_CLIENT_INIT");
      break;
    case ESP_SEND_DATA:
      Serial.println("Got ESP_SEND_DATA");
      break;
    case ESP_WIFI_CONNECT:
      Serial.println("Got ESP_WIFI_CONNECT");
      client_wifi_cnct();
      break;
    case ESP_IS_CONNECTED:
      Serial.println("Got ESP_IS_CONNECTED");
      break;
    case ESP_GET_CONNECTED_LIST:
      Serial.println("Got ESP_GET_CONNECTED_LIST");
      break;
    case ESP_NOP:
      Serial.println("Got ESP_NOP");
      break;
    case ESP_ACK:
      Serial.println("Get ESP_ACK");
      break;
    case ESP_NOACK:
      Serial.println("Get ESP_NOACK");
      break;
    default:
      Serial.println("GOT A WEIRD ASS COMMAND");
      //TODO: LET PI KNOW THAT IT WILL NEED TO RETRANSMIT
      clean();
  }
  clean();
}

void setup() {
  Serial.begin(115200);
  Serial.println("\nstarted serial!\n");
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);

  mySerial.begin(9600);

  //Serial.printf("size of cmnd: %d \n", sizeof(esp_cmnd_pckt));
  // setup from Pi buffer
   from_pi->runRdy= 0;
  from_pi->buff = NULL;
  from_pi-> numPckts = 0;
  from_pi->curPckts = 0;
  from_pi-> cmnd = 0;
  // probs dont need to malloc, was just for sanity earlier.
  from_pi->cmnd_pckt = (esp_cmnd_pckt*)malloc(sizeof(esp_cmnd_pckt));

  WiFi.mode(WIFI_STA); 
}

int i = 0;
void loop() {
  //Serial.println(mySerial.available());
  
  if(from_pi->runRdy) return runCmnd();
  if(mySerial.available() > 31){
    Serial.println("got packet");
   return parseNreadPckt();
  }
  //if(i%500000 == 0){
   // Serial.println("writing message");
  //write_msg_pi(0b1111);
  //}
//  i++;
}