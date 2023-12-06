// BinDayIndicator (BinDicator) ESP8266 Code
// Code and tutorials written by Darren Tarbard 
// https://www.youtube.com/channel/UC44BUDSGzAjDnop3DvhAAog or https://twitter.com/tarbard
// Original code was not provided in text format, this file written by Cs2000
 
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <Adafruit_NeoPixel.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

 
// Change to suit your number of pixels and output pin on the ESP8266
#define PIN D3
#define NUMPIXELS 2
 
// Change to add in your WiFi SSID, Password & NODE_RED Server IP
const char* NODE_RED_HOST_IP = "YourNodeRedServerIPHere";
const char* ssid = "YourSdidHere";
const char* password = "YourWifiPasswordHere";
 
// Adafruit NeoPixel initilisation string
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
 
// Define some standard colours for easier reference later
const uint32_t redPixel = pixels.Color(255, 0, 0);
const uint32_t greenPixel = pixels.Color(0, 255, 0);
const uint32_t bluePixel = pixels.Color(0, 0, 255);
const uint32_t whitePixel = pixels.Color(255, 255, 255);
const uint32_t unlitPixel = pixels.Color(0, 0, 0);
const uint32_t dimWhitePixel = pixels.Color(255, 255, 255);
 
 
void setup() {
  // Boot up ESP8266, setup serial comms and try to connect to wifi
  Serial.begin(115200);
  Serial.println("Booting Up");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection to WiFi Failed! Retrying...");
    delay(5000);
  }
 
  // Set LED colour to dim white when booted and connected to your wifi with a valid IP, print the IP to serial
  Serial.print("IP Address:");
  Serial.println(WiFi.localIP());
  pixels.begin();
  pixels.setPixelColor(0, dimWhitePixel);
  pixels.show();
}
 
void loop() {
  // Connect to the NODE_RED server and pull a result, show red LED if this fails
  HTTPClient http;
  const int httpPort = 1880;
  // Connect to the NODE_RED bins webpage, change to suit your IP and Bins webpage address
  if (! http.begin("http://192.168.0.20:1880/bins")) {
    Serial.println("Connection Failed To " + String(NODE_RED_HOST_IP));
    // If we cant connect, show 1 pixel red to indicate an error
    pixels.setPixelColor(0, redPixel);
  } else {
    // If we can connect to NODE_RED, start a pull of data from NODE_RED server
    Serial.print("Doing Get: ");
    int httpCode = http.GET();
    
    Serial.printf("[HTTP] GET... code: %d\n", httpCode);
 
    // Check we're getting a HTTP Code 200
    if (httpCode = HTTP_CODE_OK) {
      String parseMe = http.getString();
      //Create a memory buffer to store the JSON data
      StaticJsonBuffer<300> JSONBuffer;
      JsonArray& parsed = JSONBuffer.parseArray(parseMe);
      
      // If we can connect to NODE_RED, but dont understand the returned data, set 1 pixel to red and output to serial
      if (!parsed.success()) {
        Serial.println("Parsing JSON Failed");
        pixels.setPixelColor(0, redPixel);
        pixels.show();
        delay(5000);
        return;
      }
    
      int arraySize = parsed.size();
      int pixelIndex = 0;
 
      static uint32_t lastColour = unlitPixel;
      for (int i = 0; i < arraySize; i++) {
        char jsonChar[10];
        parsed[i].printTo((char*)jsonChar, parsed.measureLength() + 1);
 
        // Parse retreived data and check for for matching colours. If webpage says Green, show Green LED, if Red, show Red, etc
        if (strcmp(jsonChar, "green") == 0 ) {
          pixels.setPixelColor(pixelIndex, greenPixel);
          lastColour = greenPixel;
          Serial.println("Set To Green");
        }
        if (strcmp(jsonChar, "red") == 0 ) {
          pixels.setPixelColor(pixelIndex, redPixel);
          lastColour = redPixel;
          Serial.println("Set To Red");
        }
        if (strcmp(jsonChar, "blue") == 0 ) {
          pixels.setPixelColor(pixelIndex, bluePixel);
          lastColour = bluePixel;
          Serial.println("Set To Blue");
        }
        if (strcmp(jsonChar, "white") == 0 ) {
          pixels.setPixelColor(pixelIndex, whitePixel);
          lastColour = whitePixel;
          Serial.println("Set To White");
        }
        pixelIndex++;
      }
      // If only one bin value is found, repeat colour so whole bin lights up
      if (arraySize ==1) {
        for (int i = 0; i < NUMPIXELS; i++) {
          pixels.setPixelColor(pixelIndex, lastColour);
        }
      }
      
    }
    // Clean up your HTTP connection to prevent memory leaks or other weird behaviour
    Serial.println();
    Serial.println("Closing Connection");
    http.end();
    pixels.show();
    // This is how often we check the NODE_RED server. Set to something low, like 3000 for testing and perhaps 30000 or 60000 for "Production" use
    delay(30000);
  }
 
}