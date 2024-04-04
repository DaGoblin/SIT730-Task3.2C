#include "arduino_secrets.h"
#include <WiFiNINA.h>
#include <Arduino.h>
#include <hp_BH1750.h>  

//Please enter your sensitive data in the Secret tab
char ssid[] = SECRET_SSID;                // your network SSID (name)
char pass[] = SECRET_PASS;                // your network password (use for WPA, or use as key for WEP)
String apikey = SECRET_APIKEY;  
hp_BH1750 BH1750;   
WiFiClient client;
String eventName = "It's_sunny";

char   HOST_NAME[] = "maker.ifttt.com";
String PATH_NAME   = "/trigger/" + eventName + "/with/key/" + apikey; // change your EVENT-NAME and YOUR-KEY

//lux of 400 for testing simulated with phone tourch, Real sunlight bean over 20,000 lux
const float sunlightLux = 400;
bool sunlight = false;

// 1 sec poll interval for soft testing, the sun dosn't move that fast so we could poll it evey minute to avoid the status changing too quickly
const int pollInterval = 1000;


void setup() {
  // initialize WiFi connection
  WiFi.begin(ssid, pass);

  Serial.begin(9600);
  while (!Serial);

  bool avail = BH1750.begin(BH1750_TO_GROUND);// init the sensor with address pin connetcted to ground
                                              // result (bool) wil be be "false" if no sensor found
  if (!avail) {
    Serial.println("No BH1750 sensor found!");
    while (true) {};                                        
  }
}

void loop() {
  getLUX();
  delay(pollInterval);
}

void getLUX(){
  BH1750.start();   //starts a measurement
  float lux=BH1750.getLux();  //  waits until a conversion finished
  Serial.println(lux);        
  lightCheck(lux);
}

void lightCheck(float lux){
  bool sunlightCurrentStatus = sunlight;
  String sun = "off";
  
  if (lux >= sunlightLux) {
    sunlight = true;
    sun = "on";
  }
  else {
    sunlight = false;
    sun = "off";
  }
    
  if (sunlightCurrentStatus != sunlight) {
    Serial.println("Sun Changed to " + sun);
    serverSend(sun, lux);
  }
 }
void serverSend(String sun, float lux){

  String queryString = "?value1=" + sun + "&value2=" + lux;
  // connect to web server on port 80:
  if (client.connect(HOST_NAME, 80)) {
    // if connected:
    Serial.println("Connected to server");
  }
  else {// if not connected:
    Serial.println("connection failed");
  }
  // make a HTTP request:
  // send HTTP header
  client.println("GET " + PATH_NAME + queryString + " HTTP/1.1");
  client.println("Host: " + String(HOST_NAME));
  client.println("Connection: close");
  client.println(); // end HTTP header

  while (client.connected()) {
    if (client.available()) {
      // read an incoming byte from the server and print it to serial monitor:
      char c = client.read();
      Serial.print(c);
    }
  }

  // the server's disconnected, stop the client:
  client.stop();
  Serial.println();
  Serial.println("disconnected");
}
