#include <SPI.h>
#include <EthernetLarge.h>
#include <SSLClient.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>
#include "trust_anchors.h"

// Network configuration
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
char server[] = "demo.prayagedu.com"; // Updated server address
#define MYIPADDR 192,168,1,28
#define MYIPMASK 255,255,255,0
#define MYDNS 192,168,1,1
#define MYGW 192,168,1,1
const int rand_pin = A5;

// Create Ethernet and SSL clients
EthernetClient ethClient;
SSLClient sslClient(ethClient, TAs, (size_t)TAs_NUM, rand_pin);
HttpClient client(sslClient, server, 443);

void setup() {
    Serial.begin(115200);
    Serial2.begin(9600);
    delay(1000);
    Serial.println("Begin Ethernet");
    Ethernet.init(5);

    if (Ethernet.begin(mac)) {
        Serial.println("DHCP OK!");
    } else {
        Serial.println("Failed to configure Ethernet using DHCP");
        if (Ethernet.hardwareStatus() == EthernetNoHardware) {
            Serial.println("Ethernet shield was not found.");
            while (true) {
                delay(1);
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

}

void loop() {
    if(Serial2.available() > 0){

      
      String rfidTag = "";
        while (Serial2.available() > 0) {
          char incomingByte = Serial2.read();
          rfidTag += incomingByte;
          delay(10);
        }
      Serial.print("RFID Tag Read: ");
      Serial.println(rfidTag);
      post_to_server_eth(rfidTag);
    }
}
void post_to_server_eth(String rfid){
   // Prepare JSON data using ArduinoJson
    DynamicJsonDocument jsonDoc(512); // Adjust size as needed
    JsonObject root = jsonDoc.to<JsonObject>();
    root["Module"] = "publics";
    root["Page_key"] = "markRFIDAccess";
    
    JsonObject json = root.createNestedObject("JSON");
    json["RFIDCardNo"] = rfid;

    String jsonData;
    serializeJson(jsonDoc, jsonData);

    Serial.println("Sending POST request...");
    Serial.print("JSON Data: ");
    Serial.println(jsonData);

    // Connect to server and send POST request
    String path = "/index.php";
    client.beginRequest();
    client.post(path);
    client.sendHeader("Content-Type", "application/json");
    client.sendHeader("Content-Length", jsonData.length());
    client.beginBody();
    client.print(jsonData);
    client.endRequest();

    int statusCode = client.responseStatusCode();
    String response = client.responseBody();

    Serial.print("Status Code: ");
    Serial.println(statusCode);
    Serial.println("Response:");
    Serial.println(response);
}
