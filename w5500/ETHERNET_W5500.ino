#include <SPI.h>
#include <EthernetLarge.h>
#include <SSLClient.h>
#include "trust_anchors.h"
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
char server[] = "httpbin.org";
#define MYIPADDR 192,168,1,28
#define MYIPMASK 255,255,255,0
#define MYDNS 192,168,1,1
#define MYGW 192,168,1,1
const int rand_pin = A5;
EthernetClient base_client;
SSLClient client(base_client, TAs, (size_t)TAs_NUM, rand_pin);
unsigned long beginMicros, endMicros;
unsigned long byteCount = 0;
bool printWebData = true;

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("Begin Ethernet");
    Ethernet.init(5);
    if (Ethernet.begin(mac)) { // Dynamic IP setup
        Serial.println("DHCP OK!");
    }else{
        Serial.println("Failed to configure Ethernet using DHCP");
        // Check for Ethernet hardware present
        if (Ethernet.hardwareStatus() == EthernetNoHardware) {
          Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
          while (true) {
            delay(1); // do nothing, no point running without Ethernet hardware
          }
        }
        if (Ethernet.linkStatus() == LinkOFF) {
          Serial.println("Ethernet cable is not connected.");
        }

        IPAddress ip(MYIPADDR);
        IPAddress dns(MYDNS);
        IPAddress gw(MYGW);
        IPAddress sn(MYIPMASK);
        Ethernet.begin(mac, ip, dns, gw, sn);
      Serial.println("STATIC OK!");
    }
    delay(5000);


    Serial.print("Local IP : ");
    Serial.println(Ethernet.localIP());
    Serial.print("Subnet Mask : ");
    Serial.println(Ethernet.subnetMask());
    Serial.print("Gateway IP : ");
    Serial.println(Ethernet.gatewayIP());
    Serial.print("DNS Server : ");
    Serial.println(Ethernet.dnsServerIP());
    Serial.println("Ethernet Successfully Initialized");
    if (client.connect(server, 443)) {
    Serial.print("connected to ");
    client.println("GET /get HTTP/1.1");
    client.println("Host: httpbin.org");
    client.println("Connection: close");
    client.println();
  } else {
    Serial.println("connection failed");
  }
  beginMicros = micros();
}
void loop() {
  // if there are incoming bytes available
  // from the server, read them and print them:
  int len = client.available();
  if (len > 0) {
    byte buffer[80];
    if (len > 80) len = 80;
    client.read(buffer, len);
    if (printWebData) {
      Serial.write(buffer, len); // show in the serial monitor (slows some boards)
    }
    byteCount = byteCount + len;
  }
  if (!client.connected()) {
    endMicros = micros();
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();
    Serial.print("Received ");
    Serial.print(byteCount);
    Serial.print(" bytes in ");
    float seconds = (float)(endMicros - beginMicros) / 1000000.0;
    Serial.print(seconds, 4);
    float rate = (float)byteCount / seconds / 1000.0;
    Serial.print(", rate = ");
    Serial.print(rate);
    Serial.print(" kbytes/second");
    Serial.println();
    while (true) {
      delay(1);
    }
  }
}
    
    
    
