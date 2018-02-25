#ifndef SerialMonitor_h
#define SerialMonitor_h

#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <ArduinoJson.h>
#include "Extruder.h"

#define maxEncoderCount 2
#define defaultStatusMsgInterval 1000
#define defaultEnvironmentMsgInterval 5000

class SerialMonitor
{
  public:
    SerialMonitor();
    void addExtruder(Extruder *extruder);
    void addDHT(DHT *dht);
    void setStatusMsgInterval(unsigned long statusMsgIntervalMillis);
    void setEnvironmentMsgInterval(unsigned long environmentMsgIntervalMillis);
    void update();
    void enableStatusUpdate(bool enable);
    void sendStatusUpdate();
    void sendStatusUpdate(Extruder *extruder);
    void enableEnvironmentUpdate(bool enable);
    void sendEnvironmentUpdate();

  private:
    Extruder* _extruderArray[2];
    DHT* _dht;
    unsigned long _statusMsgIntervalMillis = defaultStatusMsgInterval, _lastStatusMsgMillis, _environmentMsgIntervalMillis = defaultEnvironmentMsgInterval, _lastEnvironmentMsgMillis;
    int _extruderCount = 0;
    bool _sendStatusMessage = true, _sendEnvironmentMessage = true, _sendStalledMessage = true;
};

#endif
