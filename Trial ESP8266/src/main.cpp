#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>

const char* ssid = "Rumah no 30";
const char* password = "heavyduty";

AsyncWebServer server(80);

// int LED_BUILTIN = 2;

void setup(void) {
  pinMode (LED_BUILTIN, OUTPUT);

  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Hi! I am Durotul Muntafiah.");
  });

  AsyncElegantOTA.begin(&server);    // Start ElegantOTA
  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  unsigned long waktu = millis();
  
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
  
  Serial.println(waktu);
  
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
  
}