/*
left
ADC0  PWMCOM2

right
GPIO16  relay1
GPIO15  scl
GPIO4   sda
GPIO0   relay2
GPIO2   relay3
GPIO14  DO1
GPIO12  PWMCOM1
GPIO13  Rx2
GPIO15  Tx2
GPIO3   Rx1
GPIO1   Tx1
*/

#include <Arduino.h>
#include <AHT10.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include "Nextion.h"
#include <Adafruit_ADS1X15.h>

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>

AsyncWebServer server(80);
int ssidState = 0;
// Define known network here
const char* host = "esp32";
const char* KNOWN_SSID[] = {"Galaxy-Deny", "INFORMATION CENTER", "Dasar", "Rumah no 30", "LAB-TTAT"};
const char* KNOWN_PASSWORD[] = {"bismillah", "9000000000", "liquidgas", "heavyduty", "tegangantinggi"};
const int   KNOWN_SSID_COUNT = sizeof(KNOWN_SSID) / sizeof(KNOWN_SSID[0]); // number of known networks

uint8_t readStatus = 0;
AHT10 myAHT10(AHT10_ADDRESS_0X38);

Adafruit_ADS1115 ads1;  // current measurement
Adafruit_ADS1115 ads2;  // voltage measurement
int16_t adcCurrent, adcVoltage;

int n = 100;
int currentValue = 0;
double analogCurrentValue = 0;
int voltageValue = 0;
double analogVoltageValue = 0;

const int numReadings = 3;
int readings_current[numReadings];
int readIndex_current = 0;
double total_current = 0;
double average_current = 0;
double realCurrent = 0;
int readings_voltage[numReadings];
int readIndex_voltage = 0;
double total_voltage = 0;
double average_voltage = 0;
double realVoltage = 0;

double ohm = 0;

#define RXD2 16
#define TXD2 17

#define relay1 23
#define LED 2

int currentPage = 0;
int setValue = 0;

int temperature;
int humidity;

/* Two "independant" timed events */
const long eventTime_Measurement = 1000; //in ms
const long eventTime_Update = 5; //in ms

/* When did they start the race? */
unsigned long previousTime_Measurement = 0;
unsigned long previousTime_Update = 0;

char moveDecCro[12] = "moveDecCro";
char moveDecSmo[12] = "moveDecSmo";
char moveIncCro[12] = "moveIncCro";
char moveIncSmo[12] = "moveIncSmo";
char runInc[12] = "runInc";
char runDec[12] = "runDec";
char stop[12] = "stop";
char reset[12] = "reset";

// Declare your Nextion objects - Example (page id = 0, component id = 1, component name = "b0")
// Page 0 = welcome page
NexPage page0 = NexPage(0, 0, "page0");

// Page 1 = rules page
NexPage page1     = NexPage(1, 0, "page1");
NexButton bHomeP1 = NexButton(1, 1, "bHomeP1");
NexButton bEneP1  = NexButton(1, 2, "bEneP1");
NexButton bCalP1  = NexButton(1, 3, "bCalP1");
NexButton bSetP1  = NexButton(1, 4, "bSetP1");

// Page 2 = energize page - play
NexPage page2       = NexPage(2, 0, "page2");
NexButton bHomeP2   = NexButton(2, 1, "bButtonP2");
NexButton bEneP2    = NexButton(2, 2, "bEneP2");
NexButton bCalP2    = NexButton(2, 3, "bCalP2");
NexButton bSetP2    = NexButton(2, 4, "bSetP2");
NexButton bPlayP2   = NexButton(2, 5, "bPlayP2");
NexButton bPre25P2  = NexButton(2, 16, "bPre25P2");
NexButton bPre50P2  = NexButton(2, 17, "bPre50P2");
NexButton bPre75P2  = NexButton(2, 18, "bPre75P2");
NexButton bPre99P2  = NexButton(2, 19, "bPre99P2");
NexButton bUpP2     = NexButton(2, 20, "bUpP2");
NexButton bDownP2   = NexButton(2, 21, "bDownP2");
NexButton bResetP2  = NexButton(2, 22, "bResetP2");
NexText tSVP2       = NexText(2, 15, "tSVP2");

// Page 3 = energize page - stop
NexPage page3 = NexPage(3, 0, "page3");
NexButton bHomeP3 = NexButton(3, 1, "bHomeP3");
NexButton bEneP3 = NexButton(3, 2, "bEneP3");
NexButton bCalP3 = NexButton(3, 3, "bCalP3");
NexButton bSetP3 = NexButton(3, 4, "bSetP3");
NexButton bStopP3 = NexButton(3, 5, "bStopP3");
NexButton bPre25P3 = NexButton(3, 16, "bPre25P3");
NexButton bPre50P3 = NexButton(3, 17, "bPre50P3");
NexButton bPre75P3 = NexButton(3, 18, "bPre75P3");
NexButton bPre99P3 = NexButton(3, 19, "bPre99P3");
NexButton bUpP3 = NexButton(3, 20, "bUpP3");
NexButton bDownP3 = NexButton(3, 21, "bDownP3");
NexButton bResetP3 = NexButton(3, 22, "bResetP3");
NexText tSVP3 = NexText(3, 15, "tSVP3");

// Page 4 = calibration page
NexPage page4 = NexPage(4, 0, "page4");
NexButton bHomeP4 = NexButton(4, 1, "bHomeP4");
NexButton bEneP4 = NexButton(4, 2, "bEneP4");
NexButton bCalP4 = NexButton(4, 3, "bCalP4");
NexButton bSetP4 = NexButton(4, 4, "bSetP4");

// Page 5 = setting page
NexPage page5 = NexPage(5, 0, "page5");
NexButton bHomeP5 = NexButton(5, 1, "bHomeP5");
NexButton bEneP5 = NexButton(5, 2, "bEneP5");
NexButton bCalP5 = NexButton(5, 3, "bCalP5");
NexButton bSetP5 = NexButton(5, 4, "bSetP5");
NexButton bBrightP5 = NexButton(5, 16, "bBrightP5");
NexButton bHistoryP5 = NexButton(5, 17, "bHistoryP5");
NexButton bSet3P5 = NexButton(5, 18, "bSet3P5");

// Page 6 = setting page > brightness
NexPage page6 = NexPage(6, 0, "page6");
NexButton bHomeP6 = NexButton(6, 1, "bHomeP6");
NexButton bEneP6 = NexButton(6, 2, "bEneP6");
NexButton bCalP6 = NexButton(6, 3, "bCalP6");
NexButton bSetP6 = NexButton(6, 4, "bSetP6");
NexButton bBackP6 = NexButton(6, 11, "bBackP6");

// Page 7 = setting page > update
NexPage page7 = NexPage(7, 0, "page7");
NexButton bHomeP7 = NexButton(7, 1, "bHomeP7");
NexButton bEneP7 = NexButton(7, 2, "bEneP7");
NexButton bCalP7 = NexButton(7, 3, "bCalP7");
NexButton bSetP7 = NexButton(7, 4, "bSetP7");
NexButton bBackP7 = NexButton(7, 11, "bBackP7");

// Page 8 = setting page > setting3
NexPage page8 = NexPage(8, 0, "page8");
NexButton bHomeP8 = NexButton(8, 1, "bHomeP8");
NexButton bEneP8 = NexButton(8, 2, "bEneP8");
NexButton bCalP8 = NexButton(8, 3, "bCalP8");
NexButton bSetP8 = NexButton(8, 4, "bSetP8");
NexButton bBackP8 = NexButton(8, 11, "bBackP8");

// Page 9 = manual play smooth
NexPage page9 = NexPage(9, 0 , "page9");
NexButton bHomeP9 = NexButton(9, 1, "bHomeP9");
NexButton bEneP9 = NexButton(9, 2, "bEneP9");
NexButton bCalP9 = NexButton(9, 3, "bCalP9");
NexButton bSetP9 = NexButton(9, 4, "bSetP9");
NexButton bPlayP9 = NexButton(9, 5, "bPlayP9");
NexButton bDecP9 = NexButton(9, 6, "bDecP9");
NexButton bIncP9 = NexButton(9, 7, "bIncP9");
NexButton bSmoP9 = NexButton(9, 8, "bSmoP9");
NexButton bRouP9 = NexButton(9, 9, "bRouP9");

// Page 10 = manual play rough
NexPage page10 = NexPage(10, 0 , "page10");
NexButton bHomeP10 = NexButton(10, 1, "bHomeP10");
NexButton bEneP10 = NexButton(10, 2, "bEneP10");
NexButton bCalP10 = NexButton(10, 3, "bCalP10");
NexButton bSetP10 = NexButton(10, 4, "bSetP10");
NexButton bPlayP10 = NexButton(10, 5, "bPlayP10");
NexButton bDecP10 = NexButton(10, 6, "bDecP10");
NexButton bIncP10 = NexButton(10, 7, "bIncP10");
NexButton bSmoP10 = NexButton(10, 8, "bSmoP10");
NexButton bRouP10 = NexButton(10, 9, "bRouP10");

// Page 11 = manual stop smooth
NexPage page11 = NexPage(11, 0 , "page11");
NexButton bHomeP11 = NexButton(11, 1, "bHomeP11");
NexButton bEneP11 = NexButton(11, 2, "bEneP11");
NexButton bCalP11 = NexButton(11, 3, "bCalP11");
NexButton bSetP11 = NexButton(11, 4, "bSetP11");
NexButton bStopP11 = NexButton(11, 5, "bStopP11");
NexButton bDecP11 = NexButton(11, 6, "bDecP11");
NexButton bIncP11 = NexButton(11, 7, "bIncP11");
NexButton bSmoP11 = NexButton(11, 8, "bSmoP11");
NexButton bRouP11 = NexButton(11, 9, "bRouP11");

// Page 12 = manual stop rough
NexPage page12 = NexPage(12, 0 , "page12");
NexButton bHomeP12 = NexButton(12, 1, "bHomeP12");
NexButton bEneP12 = NexButton(12, 2, "bEneP12");
NexButton bCalP12 = NexButton(12, 3, "bCalP12");
NexButton bSetP12 = NexButton(12, 4, "bSetP12");
NexButton bStopP12 = NexButton(12, 5, "bStopP12");
NexButton bDecP12 = NexButton(12, 6, "bDecP12");
NexButton bIncP12 = NexButton(12, 7, "bIncP12");
NexButton bSmoP12 = NexButton(12, 8, "bSmoP12");
NexButton bRouP12 = NexButton(12, 9, "bRouP12");

// Register a button object to the touch event list.
NexTouch *nex_listen_list[] = {
  &page0,

  &bHomeP1,
  &bEneP1,
  &bCalP1,
  &bSetP1,

  &bHomeP2,
  &bEneP2,
  &bCalP2,
  &bSetP2,
  &bPlayP2,
  &bPre25P2,
  &bPre50P2,
  &bPre75P2,
  &bPre99P2,
  &bUpP2,
  &bDownP2,
  &bResetP2,
  &tSVP2,

  &bHomeP3,
  &bEneP3,
  &bCalP3,
  &bSetP3,
  &bStopP3,
  &bPre25P3,
  &bPre50P3,
  &bPre75P3,
  &bPre99P3,
  &bUpP3,
  &bDownP3,
  &bResetP3,
  &tSVP3,

  &bHomeP4,
  &bEneP4,
  &bCalP4,
  &bSetP4,

  &bHomeP5,
  &bEneP5,
  &bCalP5,
  &bSetP5,
  &bBrightP5,
  &bHistoryP5,
  &bSet3P5,

  &bHomeP6,
  &bEneP6,
  &bCalP6,
  &bSetP6,
  &bBackP6,

  &bHomeP7,
  &bEneP7,
  &bCalP7,
  &bSetP7,
  &bBackP7,

  &bHomeP8,
  &bEneP8,
  &bCalP8,
  &bSetP8,
  &bBackP8,

  &bHomeP9,
  &bEneP9,
  &bCalP9,
  &bSetP9,
  &bPlayP9,
  &bDecP9,
  &bIncP9,
  &bSmoP9,
  &bRouP9,

  &bHomeP10,
  &bEneP10,
  &bCalP10,
  &bSetP10,
  &bPlayP10,
  &bDecP10,
  &bIncP10,
  &bSmoP10,
  &bRouP10,

  &bHomeP11,
  &bEneP11,
  &bCalP11,
  &bSetP11,
  &bStopP11,
  &bDecP11,
  &bIncP11,
  &bSmoP11,
  &bRouP11,

  &bHomeP12,
  &bEneP12,
  &bCalP12,
  &bSetP12,
  &bStopP12,
  &bDecP12,
  &bIncP12,
  &bSmoP12,
  &bRouP12,

  NULL
};

// Page 0 = welcome page
void page0PopCallback(void *ptr) {
  currentPage = 1;
}

// Page 1 = rules page
void bHomeP1PopCallback(void *ptr) {
  currentPage = 0;
}
void bEneP1PopCallback(void *ptr) {
  currentPage = 9;
}
void bCalP1PopCallback(void *ptr) {
  currentPage = 4;
}
void bSetP1PopCallback(void *ptr) {
  currentPage = 5;
}

// Page 2 = energize page - play
void bHomeP2PopCallback(void *ptr) {
  currentPage = 0;
}
void bEneP2PopCallback(void *ptr) {
  currentPage = 9;
}
void bCalP2PopCallback(void *ptr) {
  currentPage = 4;
}
void bSetP2PopCallback(void *ptr) {
  currentPage = 5;
}
void bPlayP2PopCallback(void *ptr) {
  currentPage = 3;
}
void bPre25P2Popcallback(void *ptr) {
  setValue = 25;
}
void bPre50P2Popcallback(void *ptr) {
  setValue = 50;
}
void bPre75P2Popcallback(void *ptr) {
  setValue = 75;
}
void bPre99P2Popcallback(void *ptr) {
  setValue = 99;
}
void bUpP2Popcallback(void *ptr) {
  setValue ++;
}
void bDownP2Popcallback(void *ptr) {
  setValue --;
}
void bResetP2Popcallback(void *ptr) {
  setValue = 0;
}
void tSVP2PopCallback(void *ptr) {
  setValue = 0;
}

// Page 3 = energize page - stop
void bHomeP3PopCallback(void *ptr) {
  currentPage = 0;
}
void bEneP3PopCallback(void *ptr) {
  currentPage = 9;
}
void bCalP3PopCallback(void *ptr) {
  currentPage = 4;
}
void bSetP3PopCallback(void *ptr) {
  currentPage = 5;
}
void bStopP3PopCallback(void *ptr) {
  currentPage = 2;
}
void bPre25P3Popcallback(void *ptr) {
  setValue = 25;
}
void bPre50P3Popcallback(void *ptr) {
  setValue = 50;
}
void bPre75P3Popcallback(void *ptr) {
  setValue = 75;
}
void bPre99P3Popcallback(void *ptr) {
  setValue = 99;
}
void bUpP3Popcallback(void *ptr) {
  setValue ++;
}
void bDownP3Popcallback(void *ptr) {
  setValue --;
}
void bResetP3Popcallback(void *ptr) {
  setValue = 0;
}
void tSVP3PopCallback(void *ptr) {
  setValue = 0;
}

// Page 4 = calibration page
void bHomeP4PopCallback(void *ptr) {
  currentPage = 0;
}
void bEneP4PopCallback(void *ptr) {
  currentPage = 9;
}
void bCalP4PopCallback(void *ptr) {
  currentPage = 4;
}
void bSetP4PopCallback(void *ptr) {
  currentPage = 5;
}

// Page 5 = setting page
void bHomeP5PopCallback(void *ptr) {
  currentPage = 0;
}
void bEneP5PopCallback(void *ptr) {
  currentPage = 9;
}
void bCalP5PopCallback(void *ptr) {
  currentPage = 4;
}
void bSetP5PopCallback(void *ptr) {
  currentPage = 5;
}
void bBrightP5PopCallback(void *ptr) {
  currentPage = 6;
}
void bHistoryP5PopCallback(void *ptr) {
  currentPage = 7;
}
void bSet3P5PopCallback(void *ptr) {
  currentPage = 8;
}

// Page 6 = setting page > brightness
void bHomeP6PopCallback(void *ptr) {
  currentPage = 0;
}
void bEneP6PopCallback(void *ptr) {
  currentPage = 9;
}
void bCalP6PopCallback(void *ptr) {
  currentPage = 4;
}
void bSetP6PopCallback(void *ptr) {
  currentPage = 5;
}
void bBackP6PopCallback(void *ptr) {
  currentPage = 5;
}

// Page 7 = setting page > update
void bHomeP7PopCallback(void *ptr) {
  currentPage = 0;
}
void bEneP7PopCallback(void *ptr) {
  currentPage = 9;
}
void bCalP7PopCallback(void *ptr) {
  currentPage = 4;
}
void bSetP7PopCallback(void *ptr) {
  currentPage = 5;
}
void bBackP7PopCallback(void *ptr) {
  currentPage = 5;
}

// Page 8 = setting page > setting3
void bHomeP8PopCallback(void *ptr) {
  currentPage = 0;
}
void bEneP8PopCallback(void *ptr) {
  currentPage = 9;
}
void bCalP8PopCallback(void *ptr) {
  currentPage = 4;
}
void bSetP8PopCallback(void *ptr) {
  currentPage = 5;
}
void bBackP8PopCallback(void *ptr) {
  currentPage = 5;
}

// Page 9 = manual play smooth
void bHomeP9PopCallback(void *ptr) {
  currentPage = 0;
}
void bEneP9PopCallback(void *ptr) {
  currentPage = 2;
}
void bCalP9PopCallback(void *ptr) {
  currentPage = 4;
}
void bSetP9PopCallback(void *ptr) {
  currentPage = 5;
}
void bPlayP9PopCallback(void *ptr) {
  currentPage = 11;
}
void bDecP9PopCallback(void *ptr) {
  Serial.write(moveDecSmo, 12);
  Serial.write(stop, 12);
  // Serial.write(7);
}
void bIncP9PopCallback(void *ptr) {
  Serial.write(moveIncSmo, 12);
  Serial.write(stop, 12);
  // Serial.write(9);
}
void bSmoP9PopCallback(void *ptr) {}
void bRouP9PopCallback(void *ptr) {
  currentPage = 10;
}

// Page 10 = manual play rough
void bHomeP10PopCallback(void *ptr) {
  currentPage = 0;
}
void bEneP10PopCallback(void *ptr) {
  currentPage = 2;
}
void bCalP10PopCallback(void *ptr) {
  currentPage = 4;
}
void bSetP10PopCallback(void *ptr) {
  currentPage = 5;
}
void bPlayP10PopCallback(void *ptr) {
  currentPage = 11;
}
void bDecP10PopCallback(void *ptr) {
  Serial.write(moveDecCro, 12);
  Serial.write(stop, 12);
  // Serial.write(6);
}
void bIncP10PopCallback(void *ptr) {
  Serial.write(moveIncCro, 12);
  Serial.write(stop, 12);
  // Serial.write(8);
}
void bSmoP10PopCallback(void *ptr) {
  currentPage = 9;
}
void bRouP10PopCallback(void *ptr) {}

// Page 11 = manual stop smooth
void bHomeP11PopCallback(void *ptr) {
  currentPage = 0;
}
void bEneP11PopCallback(void *ptr) {
  currentPage = 2;
}
void bCalP11PopCallback(void *ptr) {
  currentPage = 4;
}
void bSetP11PopCallback(void *ptr) {
  currentPage = 5;
}
void bStopP11PopCallback(void *ptr) {
  currentPage = 9;
}
void bDecP11PopCallback(void *ptr) {
  Serial.write(moveDecSmo, 12);
  Serial.write(stop, 12);
  // Serial.write(7);
}
void bIncP11PopCallback(void *ptr) {
  Serial.write(moveIncSmo, 12);
  Serial.write(stop, 12);
  // Serial.write(9);
}
void bSmoP11PopCallback(void *ptr) {}
void bRouP11PopCallback(void *ptr) {
  currentPage = 12;
}

// Page 12 = manual stop rough
void bHomeP12PopCallback(void *ptr) {
  currentPage = 0;
}
void bEneP12PopCallback(void *ptr) {
  currentPage = 2;
}
void bCalP12PopCallback(void *ptr) {
  currentPage = 4;
}
void bSetP12PopCallback(void *ptr) {
  currentPage = 5;
}
void bStopP12PopCallback(void *ptr) {
  currentPage = 9;
}
void bDecP12PopCallback(void *ptr) {
  Serial.write(moveDecCro, 12);
  Serial.write(stop, 12);
  // Serial.write(6);
}
void bIncP12PopCallback(void *ptr) {
  Serial.write(moveIncCro, 12);
  Serial.write(stop, 12);
  // Serial.write(8);
}
void bSmoP12PopCallback(void *ptr) {
  currentPage = 11;
}
void bRouP12PopCallback(void *ptr) {}

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);

  myAHT10.begin();
  ads1.setGain(GAIN_TWO);
  ads2.setGain(GAIN_EIGHT);
  ads1.begin(0x48);
  ads2.begin(0x49);

  pinMode(LED, OUTPUT);
  pinMode(relay1, OUTPUT);

  currentPage = 0;

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////
  boolean wifiFound = false;
  int i, n;

  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_AP);
  WiFi.disconnect();
  delay(100);
  Serial.println("Setup done");

  // WiFi.scanNetworks will return the number of networks found
  Serial.println(F("scan start"));
  int nbVisibleNetworks = WiFi.scanNetworks();
  Serial.println(F("scan done"));
  if (nbVisibleNetworks == 0) {
    Serial.println(F("no networks found. Reset to try again"));
    while (true); // no need to go further, hang in there, will auto launch the Soft WDT reset
  }

  // if you arrive here at least some networks are visible
  Serial.print(nbVisibleNetworks);
  Serial.println(" network(s) found");

  // check if we recognize one by comparing the visible networks one by one with our list of known networks
  for (i = 0; i < nbVisibleNetworks; ++i) {
    Serial.println(WiFi.SSID(i)); // Print current SSID
    for (n = 0; n < KNOWN_SSID_COUNT; n++) { // walk through the list of known SSID and check for a match
      if (strcmp(KNOWN_SSID[n], WiFi.SSID(i).c_str())) {
        Serial.print(F("\tNot matching "));
        Serial.println(KNOWN_SSID[n]);
      } else { // we got a match
        wifiFound = true;
        break; // n is the network index we found
      }
    } // end for each known wifi SSID
    if (wifiFound) break; // break from the "for each visible network" loop
  } // end for each visible network

  if (!wifiFound) {
    Serial.println(F("no Known network identified. Reset to try again"));
    while (true); // no need to go further, hang in there, will auto launch the Soft WDT reset
  }

  // if you arrive here you found 1 known SSID
  Serial.print(F("\nConnecting to "));
  Serial.println(KNOWN_SSID[n]);

  // We try to connect to the WiFi network we found
  WiFi.begin(KNOWN_SSID[n], KNOWN_PASSWORD[n]);
  
  if (KNOWN_SSID[n] == "Galaxy-Deny"){
    ssidState = 1;
  } else if (KNOWN_SSID[n] == "INFORMATION CENTER"){
    ssidState = 2;
  } else if (KNOWN_SSID[n] == "Dasar"){
    ssidState = 3;
  } else if (KNOWN_SSID[n] == "Rumah No 30"){
    ssidState = 4;
  } else if (KNOWN_SSID[n] == "LAB-TTAT"){
    ssidState = 5;
  } else {
    ssidState = 6;
  }

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");

  // SUCCESS, you are connected to the known WiFi network
  Serial.println(F("WiFi connected, your IP address is "));
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Hi! I am ESP32 for Main Controller.");
  });

  AsyncElegantOTA.begin(&server);    // Start ElegantOTA
  server.begin();
  Serial.println("HTTP server started");

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////

  nexInit();
  // Register the pop event callback function of the components
  // Page 0 = welcome page
  page0.attachPop(page0PopCallback, &page0);
  // Page 1 = rules page
  bHomeP1.attachPop(bHomeP1PopCallback, &bHomeP1);
  bEneP1.attachPop(bEneP1PopCallback, &bEneP1);
  bCalP1.attachPop(bCalP1PopCallback, &bCalP1);
  bSetP1.attachPop(bSetP1PopCallback, &bSetP1);
  // Page 2 = energize page - play
  bHomeP2.attachPop(bHomeP2PopCallback, &bHomeP2);
  bEneP2.attachPop(bEneP2PopCallback, &bEneP2);
  bCalP2.attachPop(bCalP2PopCallback, &bCalP2);
  bSetP2.attachPop(bSetP2PopCallback, &bSetP2);
  bPlayP2.attachPop(bPlayP2PopCallback, &bPlayP2);
  bPre25P2.attachPop(bPre25P2Popcallback, &bPre25P2);
  bPre50P2.attachPop(bPre50P2Popcallback, &bPre50P2);
  bPre75P2.attachPop(bPre75P2Popcallback, &bPre75P2);
  bPre99P2.attachPop(bPre99P2Popcallback, &bPre99P2);
  bUpP2.attachPop(bUpP2Popcallback, &bUpP2);
  bDownP2.attachPop(bDownP2Popcallback, &bDownP2);
  bResetP2.attachPop(bResetP2Popcallback, &bResetP2);
  tSVP2.attachPop(tSVP2PopCallback, &tSVP2);
  // Page 3 = energize page - stop
  bHomeP3.attachPop(bHomeP3PopCallback, &bHomeP3);
  bEneP3.attachPop(bEneP3PopCallback, &bEneP3);
  bCalP3.attachPop(bCalP3PopCallback, &bCalP3);
  bSetP3.attachPop(bSetP3PopCallback, &bSetP3);
  bStopP3.attachPop(bStopP3PopCallback, &bStopP3);
  bPre25P3.attachPop(bPre25P3Popcallback, &bPre25P3);
  bPre50P3.attachPop(bPre50P3Popcallback, &bPre50P3);
  bPre75P3.attachPop(bPre75P3Popcallback, &bPre75P3);
  bPre99P3.attachPop(bPre99P3Popcallback, &bPre99P3);
  bUpP3.attachPop(bUpP3Popcallback, &bUpP3);
  bDownP3.attachPop(bDownP3Popcallback, &bDownP3);
  bResetP3.attachPop(bResetP3Popcallback, &bResetP3);
  tSVP3.attachPop(tSVP3PopCallback, &tSVP3);
  // Page 4 = calibration page
  bHomeP4.attachPop(bHomeP4PopCallback, &bHomeP4);
  bEneP4.attachPop(bEneP4PopCallback, &bEneP4);
  bCalP4.attachPop(bCalP4PopCallback, &bCalP4);
  bSetP4.attachPop(bSetP4PopCallback, &bSetP4);
  // Page 5 = setting page
  bHomeP5.attachPop(bHomeP5PopCallback, &bHomeP5);
  bEneP5.attachPop(bEneP5PopCallback, &bEneP5);
  bCalP5.attachPop(bCalP5PopCallback, &bCalP5);
  bSetP5.attachPop(bSetP5PopCallback, &bSetP5);
  bBrightP5.attachPop(bBrightP5PopCallback, &bBrightP5);
  bHistoryP5.attachPop(bHistoryP5PopCallback, &bHistoryP5);
  bSet3P5.attachPop(bSet3P5PopCallback, &bSet3P5);
  // Page 6 = setting page > brightness
  bHomeP6.attachPop(bHomeP6PopCallback, &bHomeP6);
  bEneP6.attachPop(bEneP6PopCallback, &bEneP6);
  bCalP6.attachPop(bCalP6PopCallback, &bCalP6);
  bSetP6.attachPop(bSetP6PopCallback, &bSetP6);
  bBackP6.attachPop(bBackP6PopCallback, &bBackP6);
  // Page 7 = setting page > update
  bHomeP7.attachPop(bHomeP7PopCallback, &bHomeP7);
  bEneP7.attachPop(bEneP7PopCallback, &bEneP7);
  bCalP7.attachPop(bCalP7PopCallback, &bCalP7);
  bSetP7.attachPop(bSetP7PopCallback, &bSetP7);
  bBackP7.attachPop(bBackP7PopCallback, &bBackP7);
  // Page 8 = setting page > setting3
  bHomeP8.attachPop(bHomeP8PopCallback, &bHomeP8);
  bEneP8.attachPop(bEneP8PopCallback, &bEneP8);
  bCalP8.attachPop(bCalP8PopCallback, &bCalP8);
  bSetP8.attachPop(bSetP8PopCallback, &bSetP8);
  bBackP8.attachPop(bBackP8PopCallback, &bBackP8);
  // Page 9 = manual play smooth
  bHomeP9.attachPop(bHomeP9PopCallback, &bHomeP9);
  bEneP9.attachPop(bEneP9PopCallback, &bEneP9);
  bCalP9.attachPop(bCalP9PopCallback, &bCalP9);
  bSetP9.attachPop(bSetP9PopCallback, &bSetP9);
  bPlayP9.attachPop(bPlayP9PopCallback, &bPlayP9);
  bDecP9.attachPop(bDecP9PopCallback, &bDecP9);
  bIncP9.attachPop(bIncP9PopCallback, &bIncP9);
  bSmoP9.attachPop(bSmoP9PopCallback, &bSmoP9);
  bRouP9.attachPop(bRouP9PopCallback, &bRouP9);
  // Page 10 = manual play rough
  bHomeP10.attachPop(bHomeP10PopCallback, &bHomeP10);
  bEneP10.attachPop(bEneP10PopCallback, &bEneP10);
  bCalP10.attachPop(bCalP10PopCallback, &bCalP10);
  bSetP10.attachPop(bSetP10PopCallback, &bSetP10);
  bPlayP10.attachPop(bPlayP10PopCallback, &bPlayP10);
  bDecP10.attachPop(bDecP10PopCallback, &bDecP10);
  bIncP10.attachPop(bIncP10PopCallback, &bIncP10);
  bSmoP10.attachPop(bSmoP10PopCallback, &bSmoP10);
  bRouP10.attachPop(bRouP10PopCallback, &bRouP10);
  // Page 11 = manual stop smooth
  bHomeP11.attachPop(bHomeP11PopCallback, &bHomeP11);
  bEneP11.attachPop(bEneP11PopCallback, &bEneP11);
  bCalP11.attachPop(bCalP11PopCallback, &bCalP11);
  bSetP11.attachPop(bSetP11PopCallback, &bSetP11);
  bStopP11.attachPop(bStopP11PopCallback, &bStopP11);
  bDecP11.attachPop(bDecP11PopCallback, &bDecP11);
  bIncP11.attachPop(bIncP11PopCallback, &bIncP11);
  bSmoP11.attachPop(bSmoP11PopCallback, &bSmoP11);
  bRouP11.attachPop(bRouP11PopCallback, &bRouP11);
  // Page 12 = manual stop rough
  bHomeP12.attachPop(bHomeP12PopCallback, &bHomeP12);
  bEneP12.attachPop(bEneP12PopCallback, &bEneP12);
  bCalP12.attachPop(bCalP12PopCallback, &bCalP12);
  bSetP12.attachPop(bSetP12PopCallback, &bSetP12);
  bStopP12.attachPop(bStopP12PopCallback, &bStopP12);
  bDecP12.attachPop(bDecP12PopCallback, &bDecP12);
  bIncP12.attachPop(bIncP12PopCallback, &bIncP12);
  bSmoP12.attachPop(bSmoP12PopCallback, &bSmoP12);
  bRouP12.attachPop(bRouP12PopCallback, &bRouP12);

  for (int thisReading_current = 0; thisReading_current < numReadings; thisReading_current ++) {
    readings_current[thisReading_current] = 0;
  }
  for (int thisReading_voltage = 0; thisReading_voltage < numReadings; thisReading_voltage ++) {
    readings_voltage[thisReading_voltage] = 0;
  }

  digitalWrite(relay1, LOW);
}

void loop() {
  unsigned long currentTime = millis();
  
  if (currentTime - previousTime_Update >= eventTime_Update) {
    temperature = myAHT10.readTemperature();
    humidity = myAHT10.readHumidity();
    float temperatureFloat = myAHT10.readTemperature();
    float humidityFloat = myAHT10.readHumidity();
  
    Serial2.print("tTemp.txt=\"");
    Serial2.print(temperature);
    Serial2.print("\"");Serial2.write(0xff);Serial2.write(0xff);Serial2.write(0xff);
      
    Serial2.print("tHum.txt=\"");
    Serial2.print(humidity);
    Serial2.print("\"");Serial2.write(0xff);Serial2.write(0xff);Serial2.write(0xff);

    if (currentPage == 2 || currentPage == 3) {
      Serial2.print("tSV.txt=\"");
      Serial2.print(setValue);
      Serial2.print("\"");Serial2.write(0xff);Serial2.write(0xff);Serial2.write(0xff);
    }

    if (currentPage == 5) {
      Serial2.print("tSSID.txt=\"");
      if (ssidState == 1) {
        Serial2.print("Galaxy-Deny");
      } else if (ssidState == 2) {
        Serial2.print("INFORMATION CENTER");
      } else if (ssidState == 3) {
        Serial2.print("Dasar");
      } else if (ssidState == 4) {
        Serial2.print("Rumah No 30");
      } else if (ssidState == 5) {
        Serial2.print("LAB-TTAT");
      } else {
        Serial2.print("Unidentified");
      }
    Serial2.print("\"");Serial2.write(0xff);Serial2.write(0xff);Serial2.write(0xff);
        
    Serial2.print("tIP32.txt=\"");
    Serial2.print(WiFi.localIP());
    Serial2.print("\"");Serial2.write(0xff);Serial2.write(0xff);Serial2.write(0xff);

    Serial2.print("tIPwemos.txt=\"");
    Serial2.print("Farah");
    Serial2.print("\"");Serial2.write(0xff);Serial2.write(0xff);Serial2.write(0xff);

    Serial2.print("tTempFloat.txt=\"");
    Serial2.print(temperatureFloat);
    Serial2.print("\"");Serial2.write(0xff);Serial2.write(0xff);Serial2.write(0xff);

    Serial2.print("tHumFloat.txt=\"");
    Serial2.print(humidityFloat);
    Serial2.print("\"");Serial2.write(0xff);Serial2.write(0xff);Serial2.write(0xff);
    }

    AsyncElegantOTA.loop();
    nexLoop(nex_listen_list);
  }


  if (currentTime - previousTime_Measurement >= eventTime_Measurement) {
    
    int maxAnalogCurrentValue = 0;
    int maxAnalogVoltageValue = 0;

    for( int i = 0; i < n; i ++ ) {
      adcCurrent = ads1.readADC_SingleEnded(1); // current measuremnet
      currentValue = adcCurrent;
      if (currentValue > maxAnalogCurrentValue) {
        maxAnalogCurrentValue = currentValue;
      }
      adcVoltage = ads2.readADC_SingleEnded(1); // current measuremnet
      voltageValue = adcVoltage;
      if (voltageValue < maxAnalogVoltageValue) {
        maxAnalogVoltageValue = voltageValue;
      }
    }

    total_current = total_current - readings_current[readIndex_current];
    readings_current[readIndex_current] = maxAnalogCurrentValue;
    total_current = total_current + readings_current[readIndex_current];
    readIndex_current = readIndex_current + 1;

    if (readIndex_current >= numReadings) {
      readIndex_current = 0;
    }
    maxAnalogCurrentValue = total_current / numReadings;
    // 1.01*x+0.561
    if (maxAnalogCurrentValue > 6000) {
      realCurrent = 1.01*((maxAnalogCurrentValue * 0.0053) + 17.687)+0.561;
    } else {
      realCurrent = 1.01*((maxAnalogCurrentValue * 0.0075) + 4.3197)+0.561;
    }

    total_voltage = total_voltage - readings_voltage[readIndex_voltage];
    readings_voltage[readIndex_voltage] = maxAnalogVoltageValue;
    total_voltage = total_voltage + readings_voltage[readIndex_voltage];
    readIndex_voltage = readIndex_voltage + 1;

    if (readIndex_voltage >= numReadings) {
      readIndex_voltage = 0;
    }
    maxAnalogVoltageValue = -1 * total_voltage / numReadings;
    // 0.73*x-20.1
    realVoltage = 0.73*(maxAnalogVoltageValue * 0.015625)-20.1;

    ohm = realVoltage / realCurrent;
    
    if (currentPage == 3 || currentPage == 11 || currentPage == 12) {
      Serial2.print("tOhm.txt=\"");
      Serial2.print(ohm, 4);
      Serial2.print("\"");Serial2.write(0xff);Serial2.write(0xff);Serial2.write(0xff);

      Serial2.print("tAmp.txt=\"");
      // Serial2.print(maxAnalogCurrentValue);
      Serial2.print(realCurrent, 2);
      Serial2.print("\"");Serial2.write(0xff);Serial2.write(0xff);Serial2.write(0xff);
        
      Serial2.print("tVolt.txt=\"");
      // Serial2.print(maxAnalogVoltageValue);
      if (realVoltage > 99) {
        Serial2.print(realVoltage, 2);
      } else {
        Serial2.print(realVoltage, 2);
      }
      Serial2.print("\"");Serial2.write(0xff);Serial2.write(0xff);Serial2.write(0xff);
      
      digitalWrite(relay1, HIGH);
    } else {
      Serial2.print("tOhm.txt=\"");
      Serial2.print(". . . .");
      Serial2.print("\"");Serial2.write(0xff);Serial2.write(0xff);Serial2.write(0xff);

      Serial2.print("tAmp.txt=\"");
      Serial2.print(". . . .");
      Serial2.print("\"");Serial2.write(0xff);Serial2.write(0xff);Serial2.write(0xff);
        
      Serial2.print("tVolt.txt=\"");
      Serial2.print(". . . .");
      Serial2.print("\"");Serial2.write(0xff);Serial2.write(0xff);Serial2.write(0xff);

      digitalWrite(relay1, LOW);
      Serial.write(reset, 12);
    }
  }
}