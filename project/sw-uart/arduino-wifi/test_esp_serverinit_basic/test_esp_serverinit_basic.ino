#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <SoftwareSerial.h>

#define rxPin 14
#define txPin 12

SoftwareSerial mySerial = SoftwareSerial(rxPin, txPin);

//server configured to listen on port 1001
WiFiServer server(1001);

IPAddress server_init(void) {
  // credentials
  const char* ssid = "poop";
  const char* password = "password";

  // for now setup access point 
  WiFi.mode(WIFI_AP);
  Serial.println(WiFi.softAP(ssid,password) ? "Ready" : "Failed!");

  // now setup a wifi server to listen on a port. 
  server.begin();

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

void setup() {
  
  // set up serial port so its listening
  Serial.begin(115200);
  Serial.println("starting");
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);

  mySerial.begin(9600);
}

void loop() {
  if (mySerial.read() == 'a') {
    Serial.println("About to init server!!!");
    IPAddress ip = server_init();
    uint8_t lsb = ip[3];
    Serial.printf("Inited server with IP = %s, LSB = %d\n", ip.toString().c_str(), lsb);
    mySerial.write(lsb);
  }
}
