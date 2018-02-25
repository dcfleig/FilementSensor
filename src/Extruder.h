#ifndef Extruder_h
#define Extruder_h

#include "Arduino.h"
#include <Encoder.h>

#define defaultStallTimeout 5000
#define defaultIdleTimeout 100
#define defaultTickVariance 5
#define defaultStalledRepeatMillis 1000
#define defaultWheelDiameter 5
#define encoderCountPerRevolution 4096

class Extruder
{
  public:
    Extruder(int id);
    Extruder(int id, int aPin, int bPin);
    Extruder(int id, int aPin, int bPin, int stallPin, int enablePin);

    void setAPin(int pin);
    void setBPin(int pin);
    void setStalledPin(int pin);
    void setStalledLevel(bool level);
    void setEnablePin(int pin);
    void setEnableLevel(bool level);

    void enable();
    void disable();
    void restart();
    void update();

    void setReverse();
    void setVariance(int variance);
    void setStallTimeout(long timeoutMillis);
    void setStalledRepeatRate(long stallRepeatRateMillis);
    void setFilamentLimit(long filamentLimitInMMs);
    void setWheelDiameter(float wheelDiameterInMM);

    bool isEnabled();
    bool isStalled();
    bool isExtruding();
    bool isRetracting();

    int getId();
    long getTickCount();
    long getFilamentUsed();

    void setStalledCallback(void (*stalledCallback)(Extruder *extruder));
    void setDirectionChangedCallback(void (*directionChangedCallback)(Extruder *extruder));
    void setExtrudeCallback(void (*extrudeCallback)(Extruder *extruder, long count));
    void setRetractCallback(void (*retractCallback)(Extruder *extruder, long count));
    void setIdleCallback(void (*idleCallback)(Extruder *extruder));

    friend bool operator==(Extruder & lhs, Extruder & rhs);

  private:
    int _id, _aPin, _bPin, _stallPin, _enablePin, _tickVariance = defaultTickVariance, _oldDirection;
    float _wheelDiameter = defaultWheelDiameter, _filamentUsed, _filamentLimit;
    bool _isStallOutput = false, _isEnableInput = false, _reverse = false, _stalledLevel = HIGH, _enabledLevel = HIGH;
    unsigned long _stallTimeoutMillis = defaultStallTimeout, _lastTickMillis, _lastStalledMillis, _stalledRepeatRateMillis = defaultStalledRepeatMillis;
    long _lastTickCount;
    Encoder* _encoder;

    enum {
        kDisabled,
        kIdle,
        kExtruding,
        kRetracting,
        kStalled
      } _extruderState = kIdle;

    void _extruderStalled();
    void (*_stalledCallback)(Extruder *extruder);
    void (*_directionChangedCallback)(Extruder *extruder);
    void (*_extrudeCallback)(Extruder *extruder, long count);
    void (*_retractCallback)(Extruder *extruder, long count);
    void (*_idleCallback)(Extruder *extruder);
};

#endif
