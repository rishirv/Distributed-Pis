#include <SoftwareSerial.h>

#define rxPin 14
#define txPin 12

SoftwareSerial mySerial = SoftwareSerial(rxPin, txPin);

char buf[32];
int read = 0;

typedef struct esp_pckt {
    uint16_t
        _sbz:2,
        nbytes:5,
        isCmd:1,
        esp_From:4,
        esp_To:4;
    uint8_t data[30];
} esp_pckt;

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

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("\nstarted serial!\n");
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);

  mySerial.begin(9600);

}

// Test that we can receive a simple command and 1 data packet
// send_cmd(&u,ESP_ACK, 0B1010, 0B111, data, 30)
void loop() {
  
  if (mySerial.available() == 64){
    Serial.println("got some bytes!\n");
    //Serial.println("seen something at leas");
    char cmd[32], data[32];
    for (int i =0; i < 32; i++){
      cmd[i] = mySerial.read();
    }
    for (int i =0; i < 32; i++){
      data[i] = mySerial.read();
    }
    delay(100);
    int nbytes = 0;
    for (int i = 0; i < 32; i++) {
      nbytes += mySerial.write(cmd[i]);
    }
    for (int i = 0; i < 32; i++) {
      nbytes += mySerial.write(data[i]);
    }
    esp_cmnd_pckt * cmd_pkt = (esp_cmnd_pckt*) cmd;
    esp_pckt * data_pkt = (esp_pckt*) data;
    
    Serial.printf("bytes sent %d\n", nbytes);

    if (cmd_pkt->isCmd) {
      Serial.println("CMD: isCmnd checks out\n");
    }
    if (cmd_pkt->cmnd == 0b1000){
      Serial.println("CMD: command checks out\n");
    }
    if (cmd_pkt->esp_To == 0b1010){
      Serial.println("CMD: TO checks out\n");
    }
    if (cmd_pkt->esp_From == 0b1111){
      Serial.println("CMD: FROM checks out\n");
    }
    if (cmd_pkt->cksum == 0xffffffff) {
      Serial.println("CMD: Checksum checks out!\n");
    }
    if (data_pkt->isCmd == 0) {
      Serial.println("DATA: isCmnd checks out\n");      
    }
    if (data_pkt->nbytes == 30) {
      Serial.println("DATA: nbytes checks out\n");      
    }
    if (cmd_pkt->esp_To == 0b1010){
      Serial.println("DATA: TO checks out\n");
    }
    if (cmd_pkt->esp_From == 0b1111){
      Serial.println("DATA: FROM checks out\n");
    }
    //Serial.println();
  }
}