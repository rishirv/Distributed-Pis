#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <SoftwareSerial.h>

#define rxPin 14
#define txPin 12

SoftwareSerial mySerial = SoftwareSerial(rxPin, txPin);

//server configured to listen on port 1001
WiFiServer server(1001);
WiFiClient client;

enum { 
    ESP_CLIENT_INIT         = 0b0001,
    ESP_SERVER_INIT         = 0b0010,
    ESP_SEND_DATA           = 0b0011,
    ESP_WIFI_CONNECT        = 0b0100,
    ESP_IS_CONNECTED        = 0b0101,
    ESP_GET_CONNECTED_LIST  = 0b0110,
    ESP_NOP                 = 0b0111,
    ESP_ACK                 = 0b1000,
    ESP_NOACK               = 0b1001,
};

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

// Clean the state but don't through an alarming error :)
void clean(){
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
      Serial.println("I GOT A COMMAND YO!");
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
  if(from_pi->cmnd && (from_pi->numPckts == from_pi->curPckts)){
    //TODO check checksum - this might require a bit bc we arent including the headersin the chksum 
  // but I just shoved the headers in the data buffer here. 
    Serial.println("About to set runRdy to 1");
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

void sanity_print(){
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
    //read_err_pi();
}

IPAddress server_init(void) {
  // credentials
  const char* ssid = "poop";
  const char* password = "password";

  // for now setup access point 
  WiFi.mode(WIFI_AP);
  Serial.println(WiFi.softAP(ssid,password) ? "Ready" : "Failed!");
  Serial.println(WiFi.softAPIP());

  // now setup a wifi server to listen on a port. 
  server.begin();
  client = server.available();
  // then set up mdns 
  if (!MDNS.begin("poop")) {             // Start the mDNS responder for esp8266.local
    Serial.println("Error setting up MDNS responder!");
  }
  Serial.println("mDNS responder started");

  MDNS.addService("poop", "lab", 1001); // Announce service on port 1001

  int n = MDNS.queryService("poop", "lab"); // Send out query for services
  Serial.println("mDNS query done");
  if (n == 0) {
    Serial.println("no services found");
  } else {
    Serial.print(n);
    Serial.println(" service(s) found");
    for (int i = 0; i < n; ++i) {
      // Print details for each service found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(MDNS.hostname(i));
      Serial.print(" (");
      Serial.print(MDNS.IP(i));
      Serial.print(":");
      Serial.print(MDNS.port(i));
      Serial.println(")");
    }
  }
  Serial.println();
  return WiFi.softAPIP();
}

void runCmnd(){
  IPAddress ip;
  uint8_t lsb;
  switch(from_pi->cmnd) {
    case ESP_CLIENT_INIT:
      Serial.println("Got ESP_CLIENT_INIT");
      break;
    case ESP_SERVER_INIT:
      Serial.println("About to init server!!!");
      ip = server_init();
      lsb = ip[3];
      Serial.printf("Inited server with IP = %s, LSB = %d\n", ip.toString().c_str(), lsb);
      // Send the LSB of the IP address back to the pi!
      mySerial.write(lsb);
      break;      
    case ESP_SEND_DATA:
      Serial.println("Got ESP_SEND_DATA");
      break;
    case ESP_WIFI_CONNECT:
      Serial.println("Got ESP_WIFI_CONNECT");
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
      read_err_pi();
  }
  clean();
}

void setup() {
  Serial.println("IN SET UP!!!");
  // set up serial port so its listening
  Serial.begin(115200);
  Serial.println("starting");
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

void loop() {
  // check if runRdy and if so, run command!
  if(from_pi->runRdy) runCmnd();
  if(mySerial.available() > 31) parseNreadPckt();
}
