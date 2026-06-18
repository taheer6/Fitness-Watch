#include "BLETimeService.h"

BLETimeService::BLETimeService()
  : _timeService("19B10000-E8F2-537E-4F6C-D104768A1214"),
    _timeCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite, 24) {
}

bool BLETimeService::begin() {
  if (!BLE.begin()) {
    return false;
  }

  BLE.setLocalName("XIAO-TimeSync");
  BLE.setDeviceName("XIAO-TimeSync");
  BLE.setAdvertisedService(_timeService);

  _timeService.addCharacteristic(_timeCharacteristic);
  BLE.addService(_timeService);

  _timeCharacteristic.writeValue("SYNC TIME");
  BLE.advertise();

  return true;
}

void BLETimeService::poll() {
  BLE.poll();

  if (_timeCharacteristic.written()) {
    String raw = _timeCharacteristic.value();
    DateTime parsedTime;

    if (parseTimestamp(raw, parsedTime)) {
      _latestTime = parsedTime;
      _hasNewTime = true;

      BLE.disconnect();
      BLE.stopAdvertise();
    }
  }
}

bool BLETimeService::consumeSyncedTime(DateTime& outTime) {
  if (!_hasNewTime) {
    return false;
  }

  outTime = _latestTime;
  _hasNewTime = false;
  return true;
}

bool BLETimeService::parseTimestamp(const String& raw, DateTime& outTime) {
  String value = raw;
  value.trim();

  bool hasSeconds = false;

  if (value.length() == 19) {
    hasSeconds = true;
  } else if (value.length() != 16) {
    return false;
  }

  if (value.charAt(4) != '-' ||
      value.charAt(7) != '-' ||
      value.charAt(10) != ' ' ||
      value.charAt(13) != ':') {
    return false;
  }

  if (hasSeconds && value.charAt(16) != ':') {
    return false;
  }

  uint16_t year = value.substring(0, 4).toInt();
  uint8_t month = value.substring(5, 7).toInt();
  uint8_t day = value.substring(8, 10).toInt();
  uint8_t hour24 = value.substring(11, 13).toInt();
  uint8_t minute = value.substring(14, 16).toInt();
  uint8_t second = hasSeconds ? value.substring(17, 19).toInt() : 0;

  if (month < 1 || month > 12) return false;
  if (day < 1 || day > daysInMonth(year, month)) return false;
  if (hour24 > 23) return false;
  if (minute > 59) return false;
  if (second > 59) return false;

  outTime.year = year;
  outTime.month = month;
  outTime.day = day;
  outTime.hour24 = hour24;
  outTime.minute = minute;
  outTime.second = second;
  outTime.valid = true;

  return true;
}

bool BLETimeService::isLeapYear(uint16_t year) const {
  if (year % 400 == 0) return true;
  if (year % 100 == 0) return false;
  return (year % 4 == 0);
}

uint8_t BLETimeService::daysInMonth(uint16_t year, uint8_t month) const {
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