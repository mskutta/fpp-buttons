#ifdef ESP8266 || ESP32
  #define ISR_PREFIX ICACHE_RAM_ATTR
#else
  #define ISR_PREFIX
#endif

#if !(defined(ESP_NAME))
  #define ESP_NAME "buttons01" 
#endif

#include <Arduino.h>

#include <ESP8266WiFi.h> // WIFI support
#include <ESP8266mDNS.h> // For network discovery
#include <WiFiUdp.h> // OSC over UDP
#include <ArduinoOTA.h> // Updates over the air

// WiFi Manager
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> 

// I2C
#include <SPI.h>
#include <Wire.h>

// Port Expander
#include <Adafruit_MCP23017.h>

/* WIFI */
char hostname[32] = {0};

/* Port Expander */
Adafruit_MCP23017 mcp0;

/* Button */
unsigned long waitTimeout = 0;
int lastButton = 0;

void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println(F("Config Mode"));
  Serial.println(WiFi.softAPIP());
  Serial.println(myWiFiManager->getConfigPortalSSID());
}

void setup()
{
  Serial.begin(9600);
  Wire.begin(D2, D1);

  /* LED */
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  delay(1000);

  /* WiFi */
  sprintf(hostname, "%s-%06X", ESP_NAME, ESP.getChipId());
  WiFiManager wifiManager;
  wifiManager.setAPCallback(configModeCallback);
  if(!wifiManager.autoConnect(hostname)) {
    Serial.println("WiFi Connect Failed");
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(1000);
  } 

  Serial.println(hostname);
  Serial.print(F("  "));
  Serial.print(WiFi.localIP());
  Serial.print(F("  "));
  Serial.println(WiFi.macAddress());

  /* OTA */
  ArduinoOTA.setHostname(hostname);
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println(F("\nEnd"));
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) { Serial.println(F("Auth Failed")); }
    else if (error == OTA_BEGIN_ERROR) { Serial.println(F("Begin Failed")); }
    else if (error == OTA_CONNECT_ERROR) { Serial.println(F("Connect Failed")); }
    else if (error == OTA_RECEIVE_ERROR) { Serial.println(F("Receive Failed")); } 
    else if (error == OTA_END_ERROR) { Serial.println(F("End Failed")); }
  });
  ArduinoOTA.begin();

  /* Port Expander (MCP23017) */
  mcp0.begin();

  // input
  mcp0.pinMode(0, INPUT); // 1
  mcp0.pullUp(0, HIGH); 

  mcp0.pinMode(1, INPUT); // 2
  mcp0.pullUp(1, HIGH);

  mcp0.pinMode(2, INPUT); // 3
  mcp0.pullUp(2, HIGH); 

  mcp0.pinMode(3, INPUT); // 4
  mcp0.pullUp(3, HIGH); 

  mcp0.pinMode(4, INPUT); // 5
  mcp0.pullUp(4, HIGH); 

  mcp0.pinMode(5, INPUT); // 6
  mcp0.pullUp(5, HIGH); 

  mcp0.pinMode(6, INPUT); // 7
  mcp0.pullUp(6, HIGH);

  mcp0.pinMode(7, INPUT); // 8
  mcp0.pullUp(7, HIGH);

  // leds
  mcp0.pinMode(8, OUTPUT); // 1
  mcp0.digitalWrite(8, HIGH);
  
  mcp0.pinMode(9, OUTPUT); // 2
  mcp0.digitalWrite(9, HIGH);
  
  mcp0.pinMode(10, OUTPUT); // 3
  mcp0.digitalWrite(10, HIGH);
  
  mcp0.pinMode(11, OUTPUT); // 4
  mcp0.digitalWrite(11, HIGH);
  
  mcp0.pinMode(12, OUTPUT); // 5
  mcp0.digitalWrite(12, HIGH);
  
  mcp0.pinMode(13, OUTPUT); // 6
  mcp0.digitalWrite(13, HIGH);
  
  mcp0.pinMode(14, OUTPUT); // 7
  mcp0.digitalWrite(14, HIGH);
  
  mcp0.pinMode(15, OUTPUT); // 8
  mcp0.digitalWrite(15, HIGH);

  /* LED */
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
}

void TurnOnAllLeds() {
  mcp0.digitalWrite(8, HIGH); // 1
  mcp0.digitalWrite(9, HIGH); // 2
  mcp0.digitalWrite(10, HIGH); // 3
  mcp0.digitalWrite(11, HIGH); // 4
  mcp0.digitalWrite(12, HIGH); // 5
  mcp0.digitalWrite(13, HIGH); // 6
  mcp0.digitalWrite(14, HIGH); // 7
  mcp0.digitalWrite(15, HIGH); // 8
}

int getButtonPressed() {
  if (mcp0.digitalRead(0) == LOW) return 1;
  if (mcp0.digitalRead(1) == LOW) return 2;
  if (mcp0.digitalRead(2) == LOW) return 3;
  if (mcp0.digitalRead(3) == LOW) return 4;
  if (mcp0.digitalRead(4) == LOW) return 5;
  if (mcp0.digitalRead(5) == LOW) return 6;
  if (mcp0.digitalRead(6) == LOW) return 7;
  if (mcp0.digitalRead(7) == LOW) return 8;
  return 0;
}

void TurnOffAllLedsExcept(int button) {
  if (button != 1) mcp0.digitalWrite(8, LOW); // 1
  if (button != 2) mcp0.digitalWrite(9, LOW); // 2
  if (button != 3) mcp0.digitalWrite(10, LOW); // 3
  if (button != 4) mcp0.digitalWrite(11, LOW); // 4
  if (button != 5) mcp0.digitalWrite(12, LOW); // 5
  if (button != 6) mcp0.digitalWrite(13, LOW); // 6
  if (button != 7) mcp0.digitalWrite(14, LOW); // 7
  if (button != 8) mcp0.digitalWrite(15, LOW); // 8
}

void loop()
{
  ArduinoOTA.handle();
  
  if (millis() > waitTimeout)
  {
    int button = getButtonPressed();

    if (button != lastButton)
    {
      TurnOnAllLeds();
      lastButton = button;
    }

    if (button > 0)
    {
      waitTimeout = millis() + 15000;
      
      TurnOffAllLedsExcept(button);



      
    }
  }
}

