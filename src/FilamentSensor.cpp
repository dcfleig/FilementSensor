#include <Adafruit_Sensor.h>
#include <Arduino.h>
#include <DHT.h>
#include "Extruder.h"
#include "SerialMonitor.h"

#define ledPin 13
#define extruder1APin 2
#define extruder1BPin 3
#define extruder1StalledPin 8
#define extruder1EnablePin 9

#define extruder2APin 5
#define extruder2BPin 6
#define extruder2StalledPin 10
#define extruder2EnablePin 11

#define whitePin 20
#define bluePin 21
#define redPin 22
#define greenPin 23

#define dhtPin 19     // Pin which is connected to the DHT sensor.
#define dhtType DHT22 // DHT 22 (AM2302)

bool isExtruding = false;

Extruder extruder1(1, extruder1APin, extruder1BPin, extruder1StalledPin, extruder1EnablePin);
Extruder extruder2(2, extruder2APin, extruder2BPin, extruder2StalledPin, extruder2EnablePin);
DHT dht(dhtPin, dhtType);
SerialMonitor sMonitor;

void setRGBW(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
  analogWrite(whitePin, 255 - w);
  analogWrite(bluePin, 255 - r);
  analogWrite(redPin, 255 - g);
  analogWrite(greenPin, 255 - b);
}

void dirChanged(Extruder *extruder) {
}

void extruding(Extruder *extruder, long count) {
  isExtruding = true;
}

void retracting(Extruder *extruder, long count) {
  isExtruding = false;
}

void stalled(Extruder *extruder) {
  isExtruding = false;
  sMonitor.sendStatusUpdate(extruder);
}

void idle(Extruder *extruder) {
  isExtruding = false;
}


void setup() {
  Serial.begin(115200);
  Serial.println("{\"Mode\":\"Starting\"}");
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  extruder1.enable();
  extruder1.setIdleCallback(idle);
  extruder1.setExtrudeCallback(extruding);
  extruder1.setRetractCallback(retracting);
  extruder1.setStalledCallback(stalled);

//  extruder2.enable();
  extruder2.setIdleCallback(idle);
  extruder2.setExtrudeCallback(extruding);
  extruder2.setRetractCallback(retracting);
  extruder2.setStalledCallback(stalled);

  dht.begin();

  sMonitor.addExtruder(&extruder1);
  sMonitor.addExtruder(&extruder2);
  sMonitor.addDHT(&dht);

  Serial.println("{\"Mode\":\"Started\"}");
}

void loop() {
    extruder1.update();
    extruder2.update();
    sMonitor.update();
    digitalWrite(ledPin, isExtruding);
}
