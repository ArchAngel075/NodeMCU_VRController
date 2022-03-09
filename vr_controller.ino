/*
    This sketch establishes a TCP connection to a "quote of the day" service.
    It sends a "hello" message, and then prints received data.
*/

#include <ESP8266WiFi.h>
#include <TTP229.h>
#include <string>



#ifndef STASSID
#define STASSID "***"
#define STAPSK  "***"
#endif

//scl = d1 aka 20 purple
//sda = d2 aka 19 white
TTP229 ttp229;
bool keystates[8] = {0,0,0,0,0,0,0,0};

const char* ssid     = STASSID;
const char* password = STAPSK;

const char* host = "192.168.0.109";
const uint16_t port = 50000;

WiFiClient client;

void setup() {
  Wire.begin();
  Serial.begin(115200);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  /* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
     would try to act as both a client and an access-point and could cause
     network-issues with your other WiFi-devices on your WiFi-network. */
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  delay(500);
  Serial.print("connecting to ");
  Serial.print(host);
  Serial.print(':');
  Serial.println(port);

  // Use WiFiClient class to create TCP connections
  if (!client.connect(host, port)) {
    Serial.println("connection failed");
    delay(5000);
  } else {
    Serial.println("connection success");
  }
}

void loop() {
  ttp229.readKeys();
  bool isDirty = false;
  int leng = 4;
  uint8_t packet[12*leng];
  for (int i = 0; i < 12*leng; ++i) {
    packet[i] = '\0';
  }
  packet[0] = '{';
  packet[leng*1] = 'I';
  packet[leng*2] = '0';
  //packet[2*3] = '0';
  packet[leng*11] = '}';
  Serial.print("Key states: ");
  for (int i = 0; i < 8; ++i) {
      bool state = ttp229.isKeyPress(i);
      if(keystates[i] != state){
        isDirty = true;
      }
      keystates[i] = state;
      if (state) {
          Serial.print("1 ");
          packet[leng*(3+i)] = '1';
          packet[leng*(3+i)+1] = '\0';
          packet[leng*(3+i)+2] = '\0';
          packet[leng*(3+i)+3] = '\0';
      } else {
          Serial.print("0 ");
          packet[leng*(3+i)] = '0';
          packet[leng*(3+i)+1] = '\0';
          packet[leng*(3+i)+2] = '\0';
          packet[leng*(3+i)+3] = '\0';
      }
      
  }
  Serial.println();
  Serial.println("Packet built :");
  for (int i = 0; i < sizeof(packet)/sizeof(packet[0]); i++)
  {
    char buff[64];
    sprintf(buff, "send byter '%d'.",packet[i]);
    Serial.println(buff);
  }
  // This will send a string to the server
  if (client.connected()) {
    if(isDirty){
      Serial.println("sending data to server");
      client.write(packet, sizeof(packet));
      client.flush();
    }
  } else {
    // Close the connection
    Serial.println();
    Serial.println("closing connection");
    client.stop();
    return;
  }
  
  delay(1); // execute once every 5 minutes, don't flood remote service
}
