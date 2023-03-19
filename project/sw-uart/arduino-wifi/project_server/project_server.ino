
#include <hspi_slave.h>

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
//server configured to listen on port 1001
WiFiServer server(1001);
WiFiClient client;


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

}

void loop() {
  int n = 0;
  MDNS.update();
WiFiClient clientLocal = server.available(); // returns either a false value or a client with data ready to read. 
if(clientLocal) {
  Serial.println("added client");
  client = clientLocal;
}
if(client){
  Serial.println("writing");
  client.write("argh");
}




/*if (0) {
    Serial.println("wohoo got a connection");
    int avail = client.available();
    while(client.connected()){
      avail = client.available(); 
      if(avail> 0){  
        char c = client.read();
        Serial.print(c);  
        client.write('a'); 
        }  
      }
      yield();      
    }*/
}
