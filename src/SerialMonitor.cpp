#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include "SerialMonitor.h"
#include "Extruder.h"

SerialMonitor::SerialMonitor() {
  _lastStatusMsgMillis = millis();
  _lastEnvironmentMsgMillis = millis();
}

void SerialMonitor::addExtruder(Extruder *extruder) {
    _extruderArray[_extruderCount++] = extruder;
}

void SerialMonitor::addDHT(DHT *dht) {
    _dht = dht;
}

void SerialMonitor::setStatusMsgInterval(unsigned long statusMsgIntervalMillis) {
  _statusMsgIntervalMillis = statusMsgIntervalMillis;
}

void SerialMonitor::setEnvironmentMsgInterval(unsigned long environmentMsgIntervalMillis) {
  _environmentMsgIntervalMillis = environmentMsgIntervalMillis;
}

void SerialMonitor::update(){

  if ((_sendStatusMessage) && (millis() - _lastStatusMsgMillis > _statusMsgIntervalMillis)) {
    sendStatusUpdate();
    _lastStatusMsgMillis = millis();
  }

  if ((_sendEnvironmentMessage) && (millis() - _lastEnvironmentMsgMillis > _environmentMsgIntervalMillis)) {
    sendEnvironmentUpdate();
    _lastEnvironmentMsgMillis = millis();
  }
}

void SerialMonitor::enableStatusUpdate(bool enable){
  _sendStatusMessage = enable;
}

void SerialMonitor::enableEnvironmentUpdate(bool enable){
  _sendEnvironmentMessage = enable;
}

void SerialMonitor::sendStatusUpdate() {
  if (_extruderCount == 0) return;

  for (int i=0; i < _extruderCount; i++) {
    sendStatusUpdate(_extruderArray[i]);
  }
}

void SerialMonitor::sendStatusUpdate(Extruder *extruder) {
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& message = jsonBuffer.createObject();
  message["Extruder"] = extruder->getId();
  message["Enabled"] = extruder->isEnabled();
  message["Stalled"] = extruder->isStalled();
  message["Extruding"] = extruder->isExtruding();
  message["Retracting"] = extruder->isRetracting();
  message["Count"] = extruder->getTickCount();
  message["FilamentUsed"] = extruder->getFilamentUsed();
  message.printTo(Serial);
  Serial.println();
}

void SerialMonitor::sendEnvironmentUpdate() {
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& message = jsonBuffer.createObject();
  message["TemperatureC"] = _dht->readTemperature();
  message["TemperatureF"] = _dht->readTemperature(true);
  message["Humidity"] = _dht->readHumidity();
  message.printTo(Serial);
  Serial.println();
}
