#include <SoftwareSerial.h>

#define rxPin 14
#define txPin 12

SoftwareSerial mySerial = SoftwareSerial(rxPin, txPin);

char buf[32];
int read = 0;

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
