/*
 * Copyright - Himalaya vats 
 */

#include "WiFiEsp.h"
#include <ArduinoJson.h>
#include "SoftwareSerial.h"
#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>


#define DEBUG true
// Housekeeping stuff for LCD display
#define I2C_ADDR    0x3F
#define BACKLIGHT_PIN     3
#define En_pin  2
#define Rw_pin  1
#define Rs_pin  0
#define D4_pin  4
#define D5_pin  5
#define D6_pin  6
#define D7_pin  7
unsigned long lastTimeMillis = 0;
LiquidCrystal_I2C  lcd(I2C_ADDR,En_pin,Rw_pin,Rs_pin,D4_pin,D5_pin,D6_pin,D7_pin);


// Emulate Serial1 on pins 6/7 if not present
#ifndef HAVE_HWSERIAL1
SoftwareSerial Serial1(6, 7); // RX, TX
#endif

char ssid[] = "moto g";            // your network SSID (name)
char pass[] = "hvats555";        // your network password
int status = WL_IDLE_STATUS;     // the Wifi radio's status

char server[] = "himalayavats.com";

// Timer for ESP8266
unsigned long lastConnectionTime = 0;         // last time you connected to the server, in milliseconds
const unsigned long postingInterval = 10000L; // delay between updates, in milliseconds

//Timer for LCD
unsigned long lastLcdOnTime = 0;
const unsigned long lcdTimer = 180000;

struct serverResponse {
  char power[32];
};

serverResponse serverResponse;

// Initialize the Ethernet client object
WiFiEspClient client;

void setup()
{
  // initialize serial for debugging
  Serial.begin(115200);
  // initialize serial for ESP module
  Serial1.begin(9600);

  lcd.begin (16,2);
  lcd.setBacklightPin(BACKLIGHT_PIN, POSITIVE);
  lcd.setBacklight(HIGH);
  
  lcd.home();
  lcd.print("The Power Switch");
  lcd.setCursor(0,1);
  lcd.print("Please Wait...");
  
  
  // initialize ESP module
  WiFi.init(&Serial1);

  pinMode(2, OUTPUT);

  
  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue
    while (true);
  }

  // attempt to connect to WiFi network
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
  }
  lcd.clear();
  lcd.print("WiFi = OK");
}

void loop()
{
  if (millis() - lastConnectionTime > postingInterval) {
    httpRequest();
  if(skipResponseHeaders()){
    if (readReponseContent(&serverResponse)) {
      printUserData(&serverResponse);
      String power = serverResponse.power;
      applienceController(power);
         }
      }
   }
}         
void httpRequest()
{
  Serial.println();
  client.stop();

  if (client.connect(server, 80)) {
    client.println(F("GET /to_fetch/system_info.json HTTP/1.1"));
    client.println(F("Host: himalayavats.com"));
    client.println("Connection: close");
    client.println();

    lastConnectionTime = millis();
  }
  else {
    lcd.clear();
    lcd.print("Connection failed");
  }
}

bool readReponseContent(struct serverResponse* serverResponse) {
  DynamicJsonBuffer jsonBuffer(10);

  JsonObject& root = jsonBuffer.parseObject(client);

  if (!root.success()) {
    lcd.clear();
    lcd.print("JSON parsing failed!");
    return false;
  }
  strcpy(serverResponse->power, root["power"]);
  return true;
}

void printUserData(const struct serverResponse* serverResponse) {
  Serial.print("Name = ");
  //digitalWrite(2, HIGH);
  Serial.println(serverResponse->power);
}

// Skip HTTP headers so that we are at the beginning of the response's body
bool skipResponseHeaders() {
  // HTTP headers end with an empty line
  char endOfHeaders[] = "\r\n\r\n";

  client.setTimeout(10000);
  bool ok = client.find(endOfHeaders);

  if (!ok) {
    lcd.clear();
    lcd.print("No response or invalid response!");
  }

  return ok;
}

void applienceController(String power){
      if(power == "high"){
            digitalWrite(2, HIGH);
            lcd.clear();
            lcd.print("Status = ON");
            sleepLCD();
        }else if(power == "low"){
             digitalWrite(2, LOW);
             lcd.clear();
             lcd.print("OFF");
             sleepLCD();
          }     
  }

  void sleepLCD(){
        if(millis() - lastLcdOnTime > lcdTimer){
              lcd.clear();
              lcd.print("Going to Sleep");
              delay(2000);  
              lcd.setBacklight(LOW);
          }
    }
