#pragma once

#include <Arduino.h>

struct DateTime {
  uint16_t year = 2026;
  uint8_t month = 1;
  uint8_t day = 1;
  uint8_t hour24 = 0;
  uint8_t minute = 0;
  uint8_t second = 0;
  bool valid = false;
};

class TimeManager {
public:
  void begin();
  void setTime(const DateTime& dateTime);
  bool update();
  bool isRunning() const;

  const DateTime& now() const;

  void formatDate(char* out, size_t outSize) const;
  void formatTime12(char* out, size_t outSize) const;

private:
  void incrementOneSecond();
  bool isLeapYear(uint16_t year) const;
  uint8_t daysInMonth(uint16_t year, uint8_t month) const;

  DateTime _now;
  bool _running = false;
  unsigned long _lastTickMs = 0;
};
