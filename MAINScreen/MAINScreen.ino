#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include <nrf.h>
#include "TimeManager.h"
#include "BLETimeService.h"
#include "BatteryMonitor.h"

extern BLETimeService bleTime;
extern TimeManager timeManager;

// ============================================================
// Assets
// ============================================================

static const unsigned char PROGMEM image_cards_hearts_bits[] = {
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x0f,0xc0,0x0f,0xc0,0x0f,0xc0,0x0f,0xc0,0x3f,0xf0,0x3f,0xf0,0x3f,0xf0,0x3f,0xf0,
  0xff,0xfc,0xff,0xfc,0xff,0xfc,0xff,0xfc,0xff,0xff,0xff,0xfc,0xff,0xff,0xff,0xfc,
  0xff,0xff,0xff,0xfc,0xff,0xff,0xff,0xfc,0xff,0xff,0xff,0xfc,0xff,0xff,0xff,0xfc,
  0x3f,0xff,0xff,0xf0,0x3f,0xff,0xff,0xf0,0x0f,0xff,0xff,0xc0,0x0f,0xff,0xff,0xc0,
  0x03,0xff,0xff,0x00,0x03,0xff,0xff,0x00,0x00,0xff,0xfc,0x00,0x00,0xff,0xfc,0x00,
  0x00,0x3f,0xf0,0x00,0x00,0x3f,0xf0,0x00,0x00,0x0f,0xc0,0x00,0x00,0x0f,0xc0,0x00,
  0x00,0x03,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

// ============================================================
// Configuration
// ============================================================

namespace Pins {
  constexpr uint8_t TFT_CS  = D0;
  constexpr uint8_t TFT_DC  = D1;
  constexpr uint8_t TFT_RST = D2;
}

namespace DisplayConfig {
  constexpr uint16_t Width       = 240;
  constexpr uint16_t Height      = 240;
  constexpr uint32_t SpiSpeedHz  = 16000000;
  constexpr uint8_t Rotation     = 0;
  constexpr uint8_t TextSize     = 2;
}

namespace Theme {
  constexpr uint16_t Background   = ST77XX_BLACK;
  constexpr uint16_t MainText     = 0xCEC7;
  constexpr uint16_t BatteryText  = 0x3FE2;
  constexpr uint16_t HeartText    = 0xE8EC;
  constexpr uint16_t ActivityText = 0x04B1;
}

namespace Layout {
  constexpr int LeftX          = 20;
  constexpr int BatteryRightX  = 230;

  constexpr int TimeY          = 30;
  constexpr int BatteryY       = 30;
  constexpr int DateY          = 67;
  constexpr int HeartIconX     = 20;
  constexpr int HeartIconY     = 95;
  constexpr int HeartRateY     = 135;
  constexpr int StepsY         = 165;
  constexpr int DistanceY      = 198;

  constexpr int DefaultRowH    = 18;
  constexpr int TimeClearW     = 90;
  constexpr int DateClearW     = 140;
  constexpr int HeartClearW    = 60;
  constexpr int StepsClearW    = 150;
  constexpr int DistanceClearW = 100;

  constexpr int BatteryClearX  = 130;
  constexpr int BatteryClearW  = 100;
}

// ============================================================
// Model
// ============================================================

struct WatchData {
  uint8_t batteryPercent = 100;
  bool batteryCharging = false;
  bool usbPresent = false;
  float batteryVoltage = 4.2f;

  uint16_t heartRate = 110;
  uint32_t steps = 10500;
  float distanceKm = 4.6f;
};

// ============================================================
// Data Provider
// ============================================================

class WatchDataProvider {
public:
  void begin() {
    _battery.begin();
  }

  void refreshInitial(WatchData& data) {
    _battery.update();
    data.batteryPercent = _battery.percent();
    data.batteryCharging = _battery.isCharging();
    data.usbPresent = _battery.usbPresent();
    data.batteryVoltage = _battery.voltage();

    data.heartRate = getHeartRate();
    data.steps = getSteps();
    data.distanceKm = getDistanceKm();
  }

  void refreshDynamic(WatchData& data) {
    _battery.update();
    data.batteryPercent = _battery.percent();
    data.batteryCharging = _battery.isCharging();
    data.usbPresent = _battery.usbPresent();
    data.batteryVoltage = _battery.voltage();

    data.steps = getSteps();
    data.distanceKm = getDistanceKm();
  }

private:
  uint16_t getHeartRate() {
    return 110;
  }

  uint32_t getSteps() {
    return 10500;
  }

  float getDistanceKm() {
    return 4.6f;
  }

  BatteryMonitor _battery;
};

// ============================================================
// View / Renderer
// ============================================================

class WatchDisplay {
public:
  WatchDisplay() : _tft(Pins::TFT_CS, Pins::TFT_DC, Pins::TFT_RST) {}

  void begin() {
    _tft.init(DisplayConfig::Width, DisplayConfig::Height);
    _tft.setRotation(DisplayConfig::Rotation);
    _tft.setSPISpeed(DisplayConfig::SpiSpeedHz);
    _tft.setTextSize(DisplayConfig::TextSize);
    _tft.setTextWrap(false);
    _tft.fillScreen(Theme::Background);
  }

  void renderFull(const WatchData& data) {
    _tft.fillScreen(Theme::Background);
    drawDate();
    drawBattery(data);
    drawHeartIcon();
    drawHeartRate(data);
    drawSteps(data);
    drawDistance(data);
    drawTime();
  }

  void renderWaitingForBLE(const WatchData& data) {
    _tft.fillScreen(Theme::Background);
    drawLeftText(Layout::TimeY, "WAITING", Theme::MainText, 120);
    drawLeftText(Layout::DateY, "FOR BLE", Theme::MainText, 120);
    drawBattery(data);
    drawHeartIcon();
    drawHeartRate(data);
    drawSteps(data);
    drawDistance(data);
  }

  void renderLowBatteryScreen(const WatchData& data) {
    _tft.fillScreen(Theme::Background);
    drawLeftText(90, "LOW", Theme::HeartText, 80);
    drawLeftText(120, "BATTERY", Theme::HeartText, 120);

    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%.2fV", data.batteryVoltage);
    drawLeftText(155, buffer, Theme::BatteryText, 100);
  }

  void renderTimeOnly() {
    drawTime();
  }

  void renderDateOnly() {
    drawDate();
  }

  void renderBatteryOnly(const WatchData& data) {
    drawBattery(data);
  }

  void renderStepsOnly(const WatchData& data) {
    drawSteps(data);
  }

  void renderDistanceOnly(const WatchData& data) {
    drawDistance(data);
  }

private:
  void clearRect(int x, int y, int w, int h = Layout::DefaultRowH) {
    _tft.fillRect(x, y, w, h, Theme::Background);
  }

  void drawLeftText(int y, const char* text, uint16_t color, int clearWidth) {
    clearRect(Layout::LeftX, y, clearWidth);
    _tft.setTextColor(color);
    _tft.setCursor(Layout::LeftX, y);
    _tft.print(text);
  }

  void drawRightText(int y, const char* text, uint16_t color, int rightAnchorX) {
    int16_t x1, y1;
    uint16_t w, h;
    _tft.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);

    clearRect(Layout::BatteryClearX, y, Layout::BatteryClearW);
    _tft.setTextColor(color);
    _tft.setCursor(rightAnchorX - w, y);
    _tft.print(text);
  }

  void drawTime() {
    const DateTime& now = timeManager.now();

    uint8_t hour12 = 12;
    if (now.hour24 == 0) {
      hour12 = 12;
    } else if (now.hour24 > 12) {
      hour12 = now.hour24 - 12;
    } else {
      hour12 = now.hour24;
    }

    char buffer[6];
    snprintf(buffer, sizeof(buffer), "%02u:%02u", hour12, now.minute);
    drawLeftText(Layout::TimeY, buffer, Theme::MainText, Layout::TimeClearW);
  }

  void drawBattery(const WatchData& data) {
    char buffer[12];

    if (data.batteryCharging) {
      snprintf(buffer, sizeof(buffer), "%u%%+", data.batteryPercent);
    } else {
      snprintf(buffer, sizeof(buffer), "%u%%", data.batteryPercent);
    }

    drawRightText(Layout::BatteryY, buffer, Theme::BatteryText, Layout::BatteryRightX);
  }

  void drawDate() {
    const DateTime& now = timeManager.now();

    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%04u-%02u-%02u", now.year, now.month, now.day);
    drawLeftText(Layout::DateY, buffer, Theme::MainText, Layout::DateClearW);
  }

  void drawHeartIcon() {
    _tft.drawBitmap(
      Layout::HeartIconX,
      Layout::HeartIconY,
      image_cards_hearts_bits,
      30,
      32,
      Theme::HeartText
    );
  }

  void drawHeartRate(const WatchData& data) {
    char buffer[8];
    snprintf(buffer, sizeof(buffer), "%u", data.heartRate);
    drawLeftText(Layout::HeartRateY, buffer, Theme::HeartText, Layout::HeartClearW);
  }

  void drawSteps(const WatchData& data) {
    char buffer[20];
    snprintf(buffer, sizeof(buffer), "%lu Steps", static_cast<unsigned long>(data.steps));
    drawLeftText(Layout::StepsY, buffer, Theme::ActivityText, Layout::StepsClearW);
  }

  void drawDistance(const WatchData& data) {
    int whole = static_cast<int>(data.distanceKm);
    int tenth = static_cast<int>(data.distanceKm * 10.0f) % 10;

    char output[20];
    snprintf(output, sizeof(output), "%d.%d KM", whole, tenth);
    drawLeftText(Layout::DistanceY, output, Theme::ActivityText, Layout::DistanceClearW);
  }

  Adafruit_ST7789 _tft;
};

// ============================================================
// Application Controller
// ============================================================

class WatchApp {
public:
  void setup() {
    _display.begin();
    _provider.begin();
    _provider.refreshInitial(_data);

    timeManager.begin();

    if (!bleTime.begin()) {
      while (true) {
        delay(100);
      }
    }

    _lastDisplayedBatteryPercent = _data.batteryPercent;
    _lastDisplayedCharging = _data.batteryCharging;

    _display.renderWaitingForBLE(_data);
  }

  void loop() {
    bleTime.poll();

    DateTime syncedTime;
    if (bleTime.consumeSyncedTime(syncedTime)) {
      timeManager.setTime(syncedTime);
      _display.renderFull(_data);

      const DateTime& now = timeManager.now();
      _lastDisplayedMinute = now.minute;
      _lastDisplayedDay = now.day;
    }

    if (_lastBatteryUpdateMs == 0 || millis() - _lastBatteryUpdateMs >= 1000) {
      _lastBatteryUpdateMs = millis();

      _provider.refreshDynamic(_data);

      if (shouldEnterLowBatterySleep()) {
        enterLowBatterySleep();
      }

      if (_data.batteryPercent != _lastDisplayedBatteryPercent ||
          _data.batteryCharging != _lastDisplayedCharging) {
        _display.renderBatteryOnly(_data);
        _lastDisplayedBatteryPercent = _data.batteryPercent;
        _lastDisplayedCharging = _data.batteryCharging;
      }
    }

    if (!timeManager.isRunning()) {
      return;
    }

    if (timeManager.update()) {
      const DateTime& now = timeManager.now();

      if (now.day != _lastDisplayedDay) {
        _display.renderDateOnly();
        _lastDisplayedDay = now.day;
      }

      if (now.minute != _lastDisplayedMinute) {
        _display.renderTimeOnly();
        _lastDisplayedMinute = now.minute;
      }
    }
  }

private:
  static constexpr float LowBatteryCutoffVoltage = 3.10f;

  void enterLowBatterySleep() {
    _display.renderLowBatteryScreen(_data);
    delay(200);
    NRF_POWER->SYSTEMOFF = 1;
    while (true) {
      __WFE();
    }
  }

  bool shouldEnterLowBatterySleep() const {
    return !_data.usbPresent && _data.batteryVoltage < LowBatteryCutoffVoltage;
  }

  WatchData _data;
  WatchDataProvider _provider;
  WatchDisplay _display;

  uint8_t _lastDisplayedMinute = 255;
  uint8_t _lastDisplayedDay = 255;

  uint8_t _lastDisplayedBatteryPercent = 255;
  bool _lastDisplayedCharging = false;
  unsigned long _lastBatteryUpdateMs = 0;
};

// globals go here
BLETimeService bleTime;
TimeManager timeManager;

WatchApp app;

void setup() {
  app.setup();
}

void loop() {
  app.loop();
}