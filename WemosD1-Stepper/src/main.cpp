/*
ADC0    = PWMCOM1
GPIO16  = DIR
GPIO14  = PUL
GPIO12  = ENA
GPIO13  = L
GPIO15  = R

GPIO01  = Rx1
GPIO03  = Tx1
GPIO05  = N/A
GPIO06  = N/A
GPIO00  = PWMCOM2
GPIO02  = SW
*/

#include <Arduino.h>
#include <AccelStepper.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>

int ssidState = 0;
// Define known network here
const char* host = "esp32";
const char* KNOWN_SSID[] = {"Galaxy-Deny", "INFORMATION CENTER"};
const char* KNOWN_PASSWORD[] = {"bismillah", "9000000000"};
const int   KNOWN_SSID_COUNT = sizeof(KNOWN_SSID) / sizeof(KNOWN_SSID[0]); // number of known networks

AsyncWebServer server(80);

// rotary encoder pinout
#define DT 13
#define CLK 15
#define SW 2

// drv8825 pinout
#define dirPin 16
#define stepPin 14
#define enaPin 12
#define motorInterfaceType 1

AccelStepper stepper(motorInterfaceType, stepPin, dirPin);
#define minLimitSwitch 1

String serialRead;
#define LED 2
#define limitSpin 5
char mystr[12]; //Initialized variable to store recieved data

void setup() {
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  pinMode(limitSpin, INPUT);
  digitalWrite(LED, LOW);

  // WiFi.mode(WIFI_STA);
  // WiFi.begin(ssid, password);
  // Serial.println("");

  boolean wifiFound = false;
  int i, n;

  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
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
    request->send(200, "text/plain", "Hi! I am Wemos D1 for Stepper Motor Controller.");
  });

  AsyncElegantOTA.begin(&server);    // Start ElegantOTA
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {


  /* TESTING */
  // if (myString == "moveDecCro") {
  //   Serial.println("Farah");
  // }
  // else if (myString == "moveDecSmo") {
  //   Serial.println("Aqila");
  // }
  // else if (myString == "moveIncCro") {
  //   Serial.println("Durotul");
  // }
  // else if (myString == "moveIncSmo") {
  //   Serial.println("Muntafiah");
  // }
  // else if (myString == "runInc") {
  //   Serial.println("Bilqis");
  // }
  // else if (myString == "runDec") {
  //   Serial.println("Latifah");
  // }
  // else if (myString == "stop") {
  //   Serial.println("Bebek");
  // }
  // else {
  //   Serial.println("BAGUS");
  // }
  
  // Serial.print("Maulidina ");
  Serial.readBytes(mystr,12);
  String myString = String(mystr);

  /* MAIN */
  if (myString == "moveDecCro") {
    // Serial.println("Farah");
    stepper.setMaxSpeed(1000);
	  stepper.setAcceleration(200);
    stepper.moveTo(-50);
    stepper.runToPosition();
  }
  /* move decrease smooth */
  else if (myString == "moveDecSmo") {
    // Serial.println("Aqila");
    stepper.setMaxSpeed(1000);
	  stepper.setAcceleration(100);
    stepper.moveTo(-5);
    stepper.runToPosition();
  }
  /* move increase croase */
  else if (myString == "moveIncCro") {
    // Serial.println("Ayam");
    stepper.setMaxSpeed(1000);
	  stepper.setAcceleration(200);
    stepper.moveTo(50);
    stepper.runToPosition();
  }
  /* move increase smooth */
  else if (myString == "moveIncSmo") {
    // Serial.println("Bebek");
    stepper.setMaxSpeed(1000);
	  stepper.setAcceleration(100);
    stepper.moveTo(5);
    stepper.runToPosition();
  }
  else {
    // Serial.println("Kerbau");
    stepper.setCurrentPosition(0);
    // stepper.stop();
  }

  int limitState = digitalRead(limitSpin);
  if (limitState == HIGH) {
    digitalWrite(LED, HIGH);
  } else {
    digitalWrite(LED, LOW);
  }

  // if (ssidState == 1) {
  //   Serial.print("Galaxy-Deny");
  // } else if (ssidState == 2) {
  //   Serial.print("INFORMATION CENTER");
  // } else if (ssidState == 3) {
  //   Serial.print("Dasar");
  // } else if (ssidState == 4) {
  //   Serial.print("Rumah No 30");
  // } else if (ssidState == 5) {
  //   Serial.print("LAB-TTAT");
  // } else {
  //   Serial.print("Unidentified");
  // }
  
  // Serial.print(", ");
  // Serial.println(WiFi.localIP());

  AsyncElegantOTA.loop();
}