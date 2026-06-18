#include "BatteryMonitor.h"
#include <nrf.h>

// XIAO nRF52840 internal battery-monitor path on Seeed mbed core
static constexpr uint8_t BATTERY_ADC_PIN = PIN_VBAT;              // P0.31
static constexpr uint8_t BATTERY_ENABLE_PIN = PIN_VBAT_ENABLE;    // P0.14
static constexpr uint8_t CHARGE_STATUS_PIN = 22;                  // P0.17
static constexpr uint8_t CHARGE_CURRENT_PIN = 21;                 // P0.13

static constexpr float ADC_REF = 2.4f;
static constexpr int ADC_MAX = 4095;

// XIAO onboard battery-sense scaling
static constexpr float R1 = 1000000.0f;
static constexpr float R2 = 510000.0f;

void BatteryMonitor::begin() {
  analogReference(AR_INTERNAL2V4);
  analogReadResolution(12);

  pinMode(CHARGE_CURRENT_PIN, OUTPUT);
  digitalWrite(CHARGE_CURRENT_PIN, HIGH); // 50mA charge current

  pinMode(BATTERY_ENABLE_PIN, OUTPUT);
  digitalWrite(BATTERY_ENABLE_PIN, LOW);

  pinMode(CHARGE_STATUS_PIN, INPUT_PULLUP);

  delay(10);
  update();
}

void BatteryMonitor::update() {
  _usbPresent = (NRF_POWER->USBREGSTATUS & POWER_USBREGSTATUS_VBUSDETECT_Msk) != 0;

  _lastVoltage = readBatteryVoltageSmoothed();

  // Only trust voltage-based % when USB is not present
  if (!_usbPresent) {
    _lastPercent = batteryPercentFromVoltage(_lastVoltage);
  }

  bool chargePinLow = (digitalRead(CHARGE_STATUS_PIN) == LOW);
  _isCharging = _usbPresent && chargePinLow;
}

bool BatteryMonitor::isCharging() const {
  return _isCharging;
}

bool BatteryMonitor::usbPresent() const {
  return _usbPresent;
}

int BatteryMonitor::percent() const {
  return _lastPercent;
}

float BatteryMonitor::voltage() const {
  return _lastVoltage;
}

float BatteryMonitor::readBatteryVoltageRaw() {
  long sum = 0;
  const int samples = 32;

  delay(10);

  for (int i = 0; i < samples; i++) {
    sum += analogRead(BATTERY_ADC_PIN);
    delay(2);
  }

  float raw = sum / (float)samples;
  float v_adc = (raw * ADC_REF) / ADC_MAX;
  float v_batt = v_adc * ((R1 + R2) / R2);
  return v_batt;
}

float BatteryMonitor::readBatteryVoltageSmoothed() {
  float v = readBatteryVoltageRaw();

  if (_batteryVoltageFiltered == 0.0f) {
    _batteryVoltageFiltered = v;
  } else {
    _batteryVoltageFiltered = 0.9f * _batteryVoltageFiltered + 0.1f * v;
  }

  return _batteryVoltageFiltered;
}

int BatteryMonitor::batteryPercentFromVoltage(float v) const {
  if (v >= 4.20f) return 100;
  if (v >= 4.00f) return 80 + (int)((v - 4.00f) * 20.0f / 0.20f);
  if (v >= 3.85f) return 60 + (int)((v - 3.85f) * 20.0f / 0.15f);
  if (v >= 3.70f) return 40 + (int)((v - 3.70f) * 20.0f / 0.15f);
  if (v >= 3.55f) return 20 + (int)((v - 3.55f) * 20.0f / 0.15f);
  if (v >= 3.30f) return (int)((v - 3.30f) * 20.0f / 0.25f);
  return 0;
}