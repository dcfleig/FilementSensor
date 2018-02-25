#include "Arduino.h"
#include "Extruder.h"
#include <Encoder.h>

Extruder::Extruder(int id) {
  _id = id;
}

Extruder::Extruder(int id, int aPin, int bPin) {
  Extruder(id, aPin, bPin, -1, -1);
}

Extruder::Extruder(int id, int aPin, int bPin, int stalledPin, int enablePin) {
  _encoder = new Encoder(aPin, bPin);

  _id = id;
  _aPin = aPin;
  _bPin = bPin;

  if (stalledPin > -1) {
    pinMode(stalledPin, OUTPUT);
    digitalWrite(stalledPin, !_stalledLevel);
    _stallPin = stalledPin;
    _isStallOutput = true;
  }

  if (enablePin > -1) {
    pinMode(enablePin, INPUT_PULLUP);
    _enablePin = enablePin;
    _isEnableInput = true;
  }

  _extruderState = kDisabled;
}

bool operator==(Extruder & lhs, Extruder & rhs) {
  return (lhs == rhs);
}

void Extruder::setAPin(int pin) {
  _aPin = pin;
}

void Extruder::setBPin(int pin){
  _bPin = pin;
}

void Extruder::setStalledPin(int pin) {
  _stallPin = pin;
  _isStallOutput = true;
}

void Extruder::setStalledLevel(bool level) {
  _stalledLevel = level;
}

void Extruder::setEnablePin(int pin) {
  _enablePin = pin;
  _isEnableInput = true;
}

void Extruder::setEnableLevel(bool level) {
    _enabledLevel = level;
}

void Extruder::enable() {
  if (_isEnableInput && (digitalRead(_enablePin) != _enabledLevel)) {
      disable();
      return;
  }
  _filamentUsed = 0;
  _lastTickMillis = millis();
  _extruderState = kIdle;
}

void Extruder::disable() {
  _extruderState = kDisabled;
  if (_isStallOutput) digitalWrite(_stallPin, !_stalledLevel);
}

void Extruder::restart() {
  if (_isEnableInput && (digitalRead(_enablePin) != _enabledLevel)) {
      disable();
      return;
  }
  _lastTickMillis = millis();
  _extruderState = kIdle;
}

void Extruder::setReverse() {
  _reverse = true;
}

void Extruder::setVariance(int variance) {
  _tickVariance = variance;
}

void Extruder::setStallTimeout(long timeoutMillis) {
  _stallTimeoutMillis = timeoutMillis;
}

void Extruder::setStalledRepeatRate(long stalledRepeatRateMillis) {
  _stalledRepeatRateMillis = stalledRepeatRateMillis;
}

void Extruder::setFilamentLimit(long filamentLimitInMMs) {
  _filamentLimit = filamentLimitInMMs;
}

void Extruder::setWheelDiameter(float wheelDiameterInMM) {
  _wheelDiameter = wheelDiameterInMM;
}

bool Extruder::isExtruding() {
  return (_extruderState == kExtruding) ? true : false;
}

bool Extruder::isRetracting() {
  return (_extruderState == kRetracting) ? true : false;
}

bool Extruder::isStalled() {
  return (_extruderState == kStalled) ? true : false;
}

bool Extruder::isEnabled() {
  return (_extruderState == kDisabled) ? false : true;
}

int Extruder::getId() {
  return _id;
}

long Extruder::getTickCount() {
  return _lastTickCount;
}

long Extruder::getFilamentUsed() {
  return _filamentUsed;
}

void Extruder::setStalledCallback(void (*stalledCallback)(Extruder *extruder)) {
  _stalledCallback = stalledCallback;
}

void Extruder::setDirectionChangedCallback(void (*directionChangedCallback)(Extruder *extruder)) {
  _directionChangedCallback = directionChangedCallback;
}

void Extruder::setExtrudeCallback(void (*extrudeCallback)(Extruder *extruder, long count)) {
  _extrudeCallback = extrudeCallback;
}

void Extruder::setRetractCallback(void (*retractCallback)(Extruder *extruder, long count)) {
  _retractCallback = retractCallback;
}

void Extruder::setIdleCallback(void (*idleCallback)(Extruder *extruder)) {
  _idleCallback = idleCallback;
}

void Extruder::update() {
  // Re-enable if the enablePin is used and reading _enabledLevel
  if ((_extruderState == kDisabled) && (_isEnableInput && (digitalRead(_enablePin) == _enabledLevel))) {
    restart();
  }

  // Re-enable if the enablePin is used and at the right level
  if ((_extruderState == kDisabled) || (_isEnableInput && (digitalRead(_enablePin) != _enabledLevel))) {
    _extruderState = kDisabled;
    return;
  }

  if (millis() - _lastTickMillis > _stallTimeoutMillis) {
    if ((_extruderState != kStalled) || ((_stalledRepeatRateMillis > 0) && (millis() - _lastStalledMillis > _stalledRepeatRateMillis))) {
      _extruderStalled();
    }
    return;
  }

  long _currentTickCount = _encoder->read();

  if (_reverse) {
    _currentTickCount = _currentTickCount * -1;
  }

  if ((_extruderState != kIdle) && (abs(_currentTickCount - _lastTickCount) <= _tickVariance) && (millis() - _lastTickMillis > defaultIdleTimeout)) {
    if (_extruderState != kIdle) {
      if (_idleCallback != NULL) _idleCallback(this);
    }
    _extruderState = kIdle;
    return;
  }

  if (_currentTickCount > _lastTickCount + _tickVariance) {
      _extruderState = kExtruding;
      _lastTickMillis = millis();

      if (_oldDirection == kRetracting) {
        _encoder->write(0);
        _currentTickCount = 0;
        if (_directionChangedCallback != NULL)_directionChangedCallback(this);
      }
      //_filamentUsed = _filamentUsed + (((_wheelDiameter * 3.14) / (long)encoderCountPerRevolution) * ((long)_currentTickCount - (long)_lastTickCount));
      _oldDirection = kExtruding;
      _lastTickCount = _currentTickCount;
      if (_extrudeCallback != NULL) _extrudeCallback(this, _currentTickCount);

    } else if (_currentTickCount < _lastTickCount - _tickVariance) {
        _lastTickMillis = millis();
        _extruderState = kRetracting;

        if (_oldDirection == kExtruding) {
          _encoder->write(0);
          _currentTickCount = 0;
          if (_directionChangedCallback != NULL) _directionChangedCallback(this);
        }
        _oldDirection = kRetracting;
        _lastTickCount = _currentTickCount;
        if (_retractCallback != NULL) _retractCallback(this, _currentTickCount);
    }
}

void Extruder::_extruderStalled() {
  _extruderState = kStalled;

  if (_isStallOutput) {
      digitalWrite(_stallPin, _stalledLevel);
  }

  _lastStalledMillis = millis();
  if (_stalledCallback != NULL) _stalledCallback(this);
}
