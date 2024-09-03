#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <WiFi.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"

#define SD_CS_PIN 5
LiquidCrystal_I2C lcd(0x27,16,2);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
const char* ssid     = "Guest_Net";
const char* password = "12345678";
int count = 0;
char card_no[11];
char card[15];
int buzzer = 4;
int led    = 13;
String rfId="";
int row = 1;
String formattedDate;
String dayStamp;
String timeStamp;
void appendFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Appending to file: %s\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("Failed to open file for appending");
        return;
    }
    if(file.print(message)){
        Serial.println("Message appended");
    } else {
        Serial.println("Append failed");
    }
    file.close();
}
void time_stamp(){
  while(!timeClient.update()) {
    timeClient.forceUpdate();
  }
  // The formattedDate comes with the following format:
  // 2018-05-28T16:00:13Z
  // We need to extract date and time
//  formattedDate = timeClient.getFormattedDate();
  //Serial.println(formattedDate);

  // Extract date
  int splitT = formattedDate.indexOf("T");
  dayStamp = formattedDate.substring(0, splitT);
  Serial.print("DATE: ");
  Serial.println(dayStamp);
  // Extract time
  timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
  Serial.print("HOUR: ");
  Serial.println(timeStamp);
  delay(1000);
}
void setup() {
  pinMode(buzzer, OUTPUT);
  pinMode(led, OUTPUT);
  Serial.begin(115200);
  Serial2.begin(9600);
   lcd.init();                      // initialize the lcd 
  lcd.init();
  // Print a message to the LCD.
  lcd.backlight();
  lcd.setCursor(5,0);
  lcd.print("UNIQUE");
  lcd.setCursor(0,1);
  lcd.print("EMBEDDED SYSTEMS");
  
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.print("CONNECTED TO WIFI NETWORK ");
 lcd.clear();
  timeClient.begin();
  timeClient.setTimeOffset(3600);
  /////////////////////
  if(!SD.begin()){
        Serial.println("Card Mount Failed");
        return;
    }
    uint8_t cardType = SD.cardType();

    if(cardType == CARD_NONE){
        Serial.println("No SD card attached");
        return;
    }

    Serial.print("SD Card Type: ");
    if(cardType == CARD_MMC){
        Serial.println("MMC");
    } else if(cardType == CARD_SD){
        Serial.println("SDSC");
    } else if(cardType == CARD_SDHC){
        Serial.println("SDHC");
    } else {
        Serial.println("UNKNOWN");
    }

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);

  /////////////////////
  delay(1000);
  lcd.clear();
}
 
void loop() {
  lcd.setCursor(3,0);
  lcd.print("ATTENDANCE");
  lcd.setCursor(4,1);
  lcd.print("SYSTEMS");
  if(Serial2.available())
   {
      count = 0;
      while(Serial2.available() && count < 12)
      {
        card_no[count] = Serial2.read();
        count++;
        delay(5);
      }
      digitalWrite(buzzer, 1);
      digitalWrite(led,    1);
      delay(50);
      digitalWrite(buzzer, 0);
      digitalWrite(led,    0);
      Serial.println(card_no);
      for(int i = 0;i<12;i++){
         card[i] = card_no[i];
        }
      rfId = String(card);
      int l = rfId.length();
      Serial.print("....");
      Serial.println(l);
      postDataToServer();
   }
}

////////////////////////////////////


///////////////////////////////////
 
void postDataToServer() {
 
  Serial.println("Posting JSON data to server...");
  // Block until we are able to connect to the WiFi access point
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;   
    http.begin("https://demo.prayagedu.com/index.php");  
    http.addHeader("Content-Type", "application/json");         
    http.addHeader("MSC", "751d31dd6b56b26b29dac2c0e1839e34");//751d31dd6b56b26b29dac2c0e1839e34
    StaticJsonDocument<200> doc;
    doc["Module"] = "Attendance";
    doc["Page_key"] = "markAttendanceRFID";
    
    JsonObject JSON = doc.createNestedObject("JSON");
    JSON["RFIDcardNo"] = rfId;
    String requestBody;
    serializeJson(doc, requestBody);
     Serial.println(requestBody);
    int httpResponseCode = http.POST(requestBody);
    if(httpResponseCode>0){
      String response = http.getString();                       
      Serial.println(httpResponseCode);   
      Serial.println(response);
      lcd.clear();
      lcd.setCursor(3,0);
      lcd.print("ATTENADANCE");
      lcd.setCursor(2,1);
      lcd.print("SUBMITTED");
      digitalWrite(led, 1);
      digitalWrite(buzzer, 1);
      delay(500);
      digitalWrite(buzzer, 0);
      digitalWrite(led, 0);
     // time_stamp();
      String am = String(row)+","+rfId+","+dayStamp+","+timeStamp+"\r\n";
      appendFile(SD, "/Attendance.csv", am.c_str());
      count = 0;
      row = row + 1;
    }
    else {
      Serial.printf("Error occurred while sending HTTP POST: %s\n");  
    }
    lcd.clear();
    lcd.setCursor(3,0);
    lcd.print("ATTENDANCE");
    lcd.setCursor(4,1);
    lcd.print("SYSTEMS");  
  }
}
