// #include <ESP8266WiFi.h>
// #include <ESP8266mDNS.h>

// WiFiClient client;
// WiFiServer wifiServer(80);

// #define MAX_NUM_CLIENTS 16
// // Modeled from: https://forum.arduino.cc/t/connecting-multiple-clients-with-esp8266/636341
// // WiFiClient *clients[MAX_NUM_CLIENTS] = {NULL};
// WiFiClient clients[MAX_NUM_CLIENTS];
// uint8_t id = 0;
  
// void setup() {
//   Serial.begin(115200);
//   Serial.print("connected to serial\n");

//   // set up being a station
//   WiFi.mode(WIFI_STA);

//   // connect to wifi 
//   const char* ssid = "poop";
//   const char* password = "password";

//   WiFi.begin(ssid, password);             // Connect to the network
//   Serial.print("Connecting to ");
//   Serial.print(ssid); Serial.println(" ...");


//   while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
//     Serial.print(".");
//     delay(1000);
//   }

//  Serial.print("Connected to WiFi. IP:");
//   Serial.println(WiFi.localIP());
   

//   // set up mdns
// if (!MDNS.begin("poop")) {             // Start the mDNS responder
//     Serial.println("Error setting up MDNS responder!");
//   }
//   Serial.println("mDNS responder started");


// MDNS.addService("poop", "lab", 80); // Announce service on port 80, port # does not matter at this moment.

// // loop to print out all other connected mdns services 
// int n = MDNS.queryService("poop", "lab"); // Send out query for esp tcp services
//   Serial.println("mDNS query done");
//   if (n == 0) {
//     Serial.println("no services found");
//   } else {
//     Serial.print(n);
//     Serial.println(" service(s) found");
//     for (int i = 0; i < n; ++i) {
//       // Print details for each service found
//       Serial.print(i + 1);
//       Serial.print(": ");
//       Serial.print(MDNS.hostname(i));
//       Serial.print(" (");
//       Serial.print(MDNS.IP(i));
//       Serial.print(":");
//       Serial.print(MDNS.port(i));
//       Serial.println(")");
//       // id = MDNS.IP(i)[3] & 0xf;
//       // clients[id] = WiFiClient(); 
//       // while(!clients[id].connect(MDNS.IP(i), MDNS.port(i)));
//     }
//   }
//   Serial.println();
//   }

// void loop() {
//   // put your main code here, to run repeatedly:
//   MDNS.update();

//   int n = MDNS.queryService("poop", "lab");
//   if (n > 0){
//     Serial.println("connecting");
//     if (!client.connected()){
//       if(!client.connect(MDNS.IP(0),MDNS.port(0))){
//         Serial.println("Could not connect");
//         return;
//       }
//     }  
//   } else {
//     Serial.println("no server");
//     return;
//   }
//   // writing to the server supposedl
//   // if(clients[id].available()) {    
//   //   char c = clients[id].read();
//   //   Serial.println(c);    
//   // }
//   if(client.available()) {    
//     char c = client.read();
//     Serial.println(c);    
//   }

    
// }

//client code 

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>

  WiFiClient client;
  WiFiServer wifiServer(80);
  
void setup() {
  Serial.begin(115200);
  Serial.print("connected to serial\n");

  // set up being a station
  WiFi.mode(WIFI_STA);

  // connect to wifi 
  const char* ssid = "poop";
  const char* password = "password";

  WiFi.begin(ssid, password);             // Connect to the network
  Serial.print("Connecting to ");
  Serial.print(ssid); Serial.println(" ...");


  while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
    Serial.print(".");
    delay(1000);
  }

 Serial.print("Connected to WiFi. IP:");
  Serial.println(WiFi.localIP());
   

  // set up mdns
if (!MDNS.begin("poop")) {             // Start the mDNS responder
    Serial.println("Error setting up MDNS responder!");
  }
  Serial.println("mDNS responder started");


MDNS.addService("poop", "lab", 80); // Announce service on port 80, port # does not matter at this moment.

// loop to print out all other connected mdns services 
int n = MDNS.queryService("poop", "lab"); // Send out query for esp tcp services
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
  // put your main code here, to run repeatedly:
    MDNS.update();
    //Serial.printf("Your IP addr is: %d\n", client.remoteIP());

int n = MDNS.queryService("poop", "lab");
if (n > 0){
  //Serial.println("connecting");
  if (!client.connected()){
    //Serial.println("Client wasn't connected");
    if(!client.connect(MDNS.IP(0),MDNS.port(0))){
      Serial.println("Could not connect");
      return;
    }
  }  
 }else return;
// writing to the server supposedl

    if(client.available()){    
    char c = client.read();
    Serial.println(c);    
    }
}