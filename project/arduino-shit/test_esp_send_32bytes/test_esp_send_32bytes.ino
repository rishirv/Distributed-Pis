#include <SoftwareSerial.h>

#define rxPin 14
#define txPin 12

SoftwareSerial mySerial = SoftwareSerial(rxPin, txPin);

char buf[32];
int i = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("\nstarted serial!\n");
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);

  mySerial.begin(9600);

}

void loop() {
  // Note: commented out code works just fine when you
  // only send one byte from pi to esp!
  /*if (mySerial.read() == 'a') {
    Serial.println("received message!\n");
    mySerial.print("abcdefghijklmnopqrstuvwxyzabcdef");
  }*/
  // The following reads in garbage and send garbage back
  if (mySerial.available()) {
    buf[i] = mySerial.read();
    Serial.println("got: ");
    Serial.println(buf[i]);
    i++;
  }
  if (i > 25) {
    Serial.println(i);
    mySerial.print("abcdefghijklmnopqrstuvwxyzabcdef");
  }
}
