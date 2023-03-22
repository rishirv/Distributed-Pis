// server code 
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>

//server configured to listen on port 1001
WiFiServer server(1001);
WiFiClient client1;
WiFiClient client2;
WiFiClient client3;
WiFiClient client4;
WiFiClient client5;
WiFiClient client6;
WiFiClient client7;
WiFiClient client8;
WiFiClient client9;
WiFiClient client10;
WiFiClient client11;
WiFiClient client12;
WiFiClient client13;
WiFiClient client14;
WiFiClient client15;

void setup() {  
  // set up serial port so its listening
  Serial.begin(115200);
  Serial.println("starting");

  // credentials
  const char* ssid = "poop";
  const char* password = "password";

  // for now setup access point 
  WiFi.mode(WIFI_AP);
  Serial.println(WiFi.softAP(ssid,password) ? "Ready" : "Failed!");
  Serial.println(WiFi.softAPIP().toString().c_str());

  // now setup a wifi server to listen on a port. 
  server.begin();
  // then set up mdns 
  if (!MDNS.begin("poop")) { // Start the mDNS responder for esp8266.local
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
  }
  Serial.println();
}

uint8_t id = 0;
uint32_t i2 = 0;
uint32_t i3 = 0;
void loop() {

  MDNS.update(); // Always have to do in loop
  WiFiClient clientLocal = server.available(); 
  if(clientLocal) {
    Serial.println("added client");
    uint8_t id = clientLocal.remoteIP()[3];
    switch(id){
      case 1:
        client1 = clientLocal;
        break;
      case 2:
        client2 = clientLocal;
        break;
      case 3:
        client3 = clientLocal;
        break;
      case 4:
        client4 = clientLocal;
        break;
      case 5:
        client5 = clientLocal;
        break;
      case 6:
        client6 = clientLocal;
        break;
      case 7:
        client7 = clientLocal;
        break;
      case 8:
        client8 = clientLocal;
        break;
      case 9:
        client9 = clientLocal;
        break;
      case 10:
        client10 = clientLocal;
        break;
      case 11:
        client11 = clientLocal;
        break;
      case 12:
        client12 = clientLocal;
        break;
      case 13:
        client13 = clientLocal;
        break;
      case 14:
        client14 = clientLocal;
        break;
      case 15:
        client15 = clientLocal;
        break;  
      default:
        Serial.println("invalid id?");
        break;
    }
  }
  if(client1 && client1.connected()) {
    Serial.println("writing to client1");
    client1.write("abby!");
  }
  if(client2 && client2.connected()) {
    Serial.printf("writing to client2: %d\n", i2++);
    client2.write("beth!");
  }
  if(client3 && client3.connected()) {
    Serial.printf("writing to client3: %d\n", i3++);
    client3.write("cathy");
  }
  if(client4 && client4.connected()) {
    Serial.println("writing to client4");
    client3.write("daisy");
  }
  if(client5 && client5.connected()) {
    Serial.println("writing to client5");
    client3.write("essie");
  } 
  if(client6 && client6.connected()) {
    Serial.println("writing to client6");
    client3.write("fran!");
  } 
  // TODO: Fill in for remaining clients
  // TODO: Perhaps add this to the switch case
  // TODO: Figure out why we are writing to clients that aren't connected

}
