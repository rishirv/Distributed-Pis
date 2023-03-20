// #include <ESP8266WiFi.h>
// #include <ESP8266mDNS.h>
// //server configured to listen on port 1001
// WiFiServer server(1001);
// WiFiClient client;

// #define MAX_NUM_CLIENTS 16
// // Modeled from: https://forum.arduino.cc/t/connecting-multiple-clients-with-esp8266/636341
// //WiFiClient clients[MAX_NUM_CLIENTS] = malloc(size_of(WiFiClient) * MAX_NUM_CLIENTS); 
// WiFiClient clients[MAX_NUM_CLIENTS]; 
// //WiFiClient *clients[MAX_NUM_CLIENTS] = {null}; 
// uint8_t id = 0;
// IPAddress ip;
// uint16_t port;

// void setup() {
//   // set up serial port so its listening
//   Serial.begin(115200);
//   Serial.println("starting");

//   // credentials
//   const char* ssid = "poop";
//   const char* password = "password";

//   // for now setup access point 
//   WiFi.mode(WIFI_AP);
//   Serial.println(WiFi.softAP(ssid,password) ? "Ready" : "Failed!");

//   // now setup a wifi server to listen on a port. 
//   server.begin();
//   client = server.available();
//   // then set up mdns 
//   if (!MDNS.begin("poop")) {             // Start the mDNS responder for esp8266.local
//     Serial.println("Error setting up MDNS responder!");
//   }
//   Serial.println("mDNS responder started");

//   MDNS.addService("poop", "lab", 1001); // Announce service on port 1001

//   int n = MDNS.queryService("poop", "lab"); // Send out query for services
//   Serial.println("mDNS query done");
//   // if (n == 0) {
//   //   Serial.println("no services found");
//   // } else {
//   while (n == 0) {
//     n = MDNS.queryService("poop", "lab");
//     Serial.println("no services found");
//   }
//     Serial.print(n);
//     Serial.println(" service(s) found");
//     /*
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
//       // Idea: create a new client with this ip addr and add it to the array 
//       // https://arduino.stackexchange.com/questions/31256/multiple-client-server-over-wifi
//       // https://stackoverflow.com/questions/60807733/esp8266-how-to-get-a-clients-ip-with-esp8266-library-v2-6-3-running-as-a-webser
//       // WiFiClient newClient = server.available();
//       // clients[i] = new WiFiClient(newClient);
//     }
//     */
    
//     //int n = MDNS.queryService("poop", "lab"); // Send out query for services
//     for (int i = 0; i < n; i++) {
//       Serial.print(i + 1);
//       Serial.print(": ");
//       Serial.print(MDNS.hostname(i));
//       Serial.print(" (");
//       Serial.print(MDNS.IP(i));
//       Serial.print(":");
//       Serial.print(MDNS.port(i));
//       Serial.println(")");
//       id = MDNS.IP(i)[3] & 0xf;
//       ip = MDNS.IP(i);
//       port = MDNS.port(i);
//       //clients[id] = new WiFiClient(); 
//       clients[id] = WiFiClient(); 
//       //while(!clients[id].connect(MDNS.IP(i), MDNS.port(i)));
//       clients[id].connect(MDNS.IP(i), MDNS.port(i));
//     }

  
//   Serial.println();

// }

// void loop() {
//   MDNS.update();
//   //Serial.println(clients[id]);
//   if (clients[id].connected()) {
//     Serial.println("writing");
//     clients[id].write("argh");
//   } else { 
//     //clients[id].connect(ip, port);
//     clients[id].write("argh");
//     //Serial.println("client not connected :(");
//   }
      
//   /*
//   WiFiClient clientLocal = server.available(); // returns either a false value or a client with data ready to read. 
//   if(clientLocal) {
//     Serial.println("added client");
//     client = clientLocal;
//   }
//   if(client){
//     Serial.println("writing");
//     client.write("argh");
//   }
//   */




// /*if (0) {
//     Serial.println("wohoo got a connection");
//     int avail = client.available();
//     while(client.connected()){
//       avail = client.available(); 
//       if(avail> 0){  
//         char c = client.read();
//         Serial.print(c);  
//         client.write('a'); 
//         }  
//       }
//       yield();      
//     }*/
// }

// server code 

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
//server configured to listen on port 1001
WiFiServer server(1001);
WiFiClient client1;
WiFiClient client2;

//#define MAX_NUM_CLIENTS 16
//WiFiClient clients[MAX_NUM_CLIENTS]; 
//WiFiClient * clients = (WiFiClient *)malloc(sizeof(WiFiClient) * MAX_NUM_CLIENTS);
//uint32_t *clients = (uint32_t*)malloc(sizeof(uint32_t) * MAX_NUM_CLIENTS); 

void setup() {
  
  // set up serial port so its listening
Serial.begin(115200);
Serial.println("starting");
//Serial.printf("sizeof(WiFiClient) = %d\n",sizeof(WiFiClient));
//Serial.println(clients[0]);

// credentials
  const char* ssid = "poop";
  const char* password = "password";

  // for now setup access point 
  WiFi.mode(WIFI_AP);
  Serial.println(WiFi.softAP(ssid,password) ? "Ready" : "Failed!");

// now setup a wifi server to listen on a port. 
server.begin();
client1 = server.available();
client2 = server.available();
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
    // for (int i = 0; i < n; ++i) {
    //   // Print details for each service found
    //   Serial.print(i + 1);
    //   Serial.print(": ");
    //   Serial.print(MDNS.hostname(i));
    //   Serial.print(" (");
    //   Serial.print(MDNS.IP(i));
    //   Serial.print(":");
    //   Serial.print(MDNS.port(i));
    //   Serial.println(")");
    // }
  }
  Serial.println();

}

uint8_t id = 0;
void loop() {
  // int n = 0;
  // MDNS.update();
  // WiFiClient clientLocal = server.available(); // returns either a false value or a client with data ready to read. 

  // if (clientLocal) {
  //   uint8_t id = clientLocal.remoteIP()[3];
  //   if (!clients[id]) {
  //     clients[id] = clientLocal; // IDK if this will work. Maybe try 
  //   }  
  //   if(clients[id]) {
  //   Serial.println("writing");
  //   clients[id].write("argh");
  //   }
  // }
   MDNS.update();
   //Serial.println("About to check clients[id] in loop");
  //  WiFiClient client = (WiFiClient)clients[id];
  //  if (client) {
  //    Serial.println("writing");
  //    //WiFiClient client = (WiFiClient)clients[id];
  //    client.write("argh");
  //  } else {
  //    Serial.println("about to server.available() in loop");
  //    WiFiClient clientLocal = server.available();
  //    if (clientLocal) {
  //      Serial.println("Found a client!");       
  //      id = clientLocal.remoteIP()[3];
  //      Serial.printf("id: %d\n", id);
  //      //if (!clients[id]) {
  //       clients[id] = WiFiClient(clientLocal); // IDK if this will work. Maybe try 
  //      //} 
  //    }
     
  //  }

  WiFiClient clientLocal = server.available(); 
  if(clientLocal) {
    Serial.println("added client");
    uint8_t id = clientLocal.remoteIP()[3];
    switch(id){
      case 3:
        client1 = clientLocal;
        break;
      case 2:
        client2 = clientLocal;
        break;
      default:
        break;
    }
  }
  if(client1 && client1.connected()){
    Serial.println("writing 1");
    client1.write("victoria");
  }
  if(client2 && client2.connected()){
    Serial.println("writing 2");
    client2.write("sarah");
  }
}

// clientLocal.remoteIP()[3]
// Use IP addr to see if client already there.



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
//}
