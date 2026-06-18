#include "TimeManager.h"

void TimeManager::begin() {
  _lastTickMs = millis();
}

void TimeManager::setTime(const DateTime& dateTime) {
  _now = dateTime;
  _running = dateTime.valid;
  _lastTickMs = millis();
}

bool TimeManager::update() {
  if (!_running) {
    return false;
  }

  unsigned long nowMs = millis();
  bool changed = false;

  while (nowMs - _lastTickMs >= 1000) {
    _lastTickMs += 1000;
    incrementOneSecond();
    changed = true;
  }

  return changed;
}

bool TimeManager::isRunning() const {
  return _running;
}

const DateTime& TimeManager::now() const {
  return _now;
}

void TimeManager::formatDate(char* out, size_t outSize) const {
  snprintf(out, outSize, "%04u-%02u-%02u", _now.year, _now.month, _now.day);
}

void TimeManager::formatTime12(char* out, size_t outSize) const {
  uint8_t hour12 = 12;
  const char* suffix = "AM";

  if (_now.hour24 == 0) {
    hour12 = 12;
    suffix = "AM";
  } else if (_now.hour24 < 12) {
    hour12 = _now.hour24;
    suffix = "AM";
  } else if (_now.hour24 == 12) {
    hour12 = 12;
    suffix = "PM";
  } else {
    hour12 = _now.hour24 - 12;
    suffix = "PM";
  }

  snprintf(out, outSize, "%02u:%02u:%02u %s", hour12, _now.minute, _now.second, suffix);
}

void TimeManager::incrementOneSecond() {
  _now.second++;

  if (_now.second < 60) {
    return;
  }

  _now.second = 0;
  _now.minute++;

  if (_now.minute < 60) {
    return;
  }

  _now.minute = 0;
  _now.hour24++;

  if (_now.hour24 < 24) {
    return;
  }

  _now.hour24 = 0;
  _now.day++;

  if (_now.day <= daysInMonth(_now.year, _now.month)) {
    return;
  }

  _now.day = 1;
  _now.month++;

  if (_now.month <= 12) {
    return;
  }

  _now.month = 1;
  _now.year++;
}

bool TimeManager::isLeapYear(uint16_t year) const {
  if (year % 400 == 0) return true;
  if (year % 100 == 0) return false;
  return (year % 4 == 0);
}

uint8_t TimeManager::daysInMonth(uint16_t year, uint8_t month) const {
  switch (month) {
    case 1: return 31;
    case 2: return isLeapYear(year) ? 29 : 28;
    case 3: return 31;
    case 4: return 30;
    case 5: return 31;
    case 6: return 30;
    case 7: return 31;
    case 8: return 31;
    case 9: return 30;
    case 10: return 31;
    case 11: return 30;
    case 12: return 31;
    default: return 30;
  }
}
