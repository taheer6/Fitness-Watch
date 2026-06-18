#pragma once

#include <Arduino.h>

class BatteryMonitor {
public:
  void begin();
  void update();

  bool isCharging() const;
  bool usbPresent() const;
  int percent() const;
  float voltage() const;

private:
  float readBatteryVoltageRaw();
  float readBatteryVoltageSmoothed();
  int batteryPercentFromVoltage(float v) const;

  float _batteryVoltageFiltered = 0.0f;
  float _lastVoltage = 0.0f;
  int _lastPercent = 0;
  bool _isCharging = false;
  bool _usbPresent = false;
};