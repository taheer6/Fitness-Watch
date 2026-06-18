#pragma once

#include <Arduino.h>
#include <ArduinoBLE.h>
#include "TimeManager.h"

class BLETimeService {
public:
  BLETimeService();

  bool begin();
  void poll();
  bool consumeSyncedTime(DateTime& outTime);

private:
  bool parseTimestamp(const String& raw, DateTime& outTime);
  uint8_t daysInMonth(uint16_t year, uint8_t month) const;
  bool isLeapYear(uint16_t year) const;

  BLEService _timeService;
  BLEStringCharacteristic _timeCharacteristic;

  bool _hasNewTime = false;
  DateTime _latestTime;
};
