#ifndef LED_DISPLAY_H
#define LED_DISPLAY_H

#include <cstdint>
#include <array>
#include <Arduino.h>

enum class ActivePolarity {
  activeHigh,
  activeLow,
};

template<ActivePolarity Polarity>
class LEDDisplay {
public:

  enum class LedState {
    ledOn,
    ledOff,
  };

  enum class Mode {
    Cool,
    Heat,
    Dry,
    Fan,
    AutoCool,
    AutoHeat,
  };

  LEDDisplay(uint8_t ledOnePin, uint8_t ledTwoPin, uint8_t ledThreePin);
  ~LEDDisplay();

  void showMode(Mode mode, bool saving);
  void showError(int errorNo);
  void enable();
  void disable();

private:

  struct ErrorStruct {
    enum class ErrorClass {
      E,
      F,
      H
    };

    static const TickType_t NORMAL_BLINK_DELAY = pdMS_TO_TICKS(500);
    static const TickType_t LONG_BLINK_DELAY = pdMS_TO_TICKS(1500);

    TimerHandle_t blinkTimer = nullptr;
    std::array<uint8_t, 3> errorLedsPins;
    uint8_t totalBlinks = 0;
    uint8_t blinkCount = 0;
    LedState errorLedState = LedState::ledOff;
    uint8_t errorLed = 0;

    LEDDisplay<Polarity>* parent = nullptr;

    static void blinkCallback(TimerHandle_t xTimer);
    void handleBlink();
  };

  static constexpr uint8_t allLedOffMask = 0x00;
  static constexpr uint8_t ledOneMask = 0x01;
  static constexpr uint8_t ledTwoMask = 0x02;
  static constexpr uint8_t ledThreeMask = 0x04;

  static constexpr uint8_t MaskCool = ledOneMask;
  static constexpr uint8_t MaskCoolSave = ledOneMask | ledThreeMask;
  static constexpr uint8_t MaskHeat = ledTwoMask;
  static constexpr uint8_t MaskHeatSave = ledTwoMask | ledThreeMask;
  static constexpr uint8_t MaskDry = ledThreeMask;
  static constexpr uint8_t MaskFan = allLedOffMask;  // Fan mode = no LEDs
  static constexpr uint8_t MaskAutoCool = ledOneMask;
  static constexpr uint8_t MaskAutoHeat = ledTwoMask;

  static constexpr std::array<uint8_t, 6> ModeMasks = {
    MaskCool, MaskHeat, MaskDry, MaskFan, MaskAutoCool, MaskAutoHeat
  };

  uint8_t ledOne;
  uint8_t ledTwo;
  uint8_t ledThree;

  ErrorStruct errorData;

  void ledTurnOnMask(uint8_t mask);
  inline void writeLed(uint8_t pin, LedState ledState) const;
};

template<ActivePolarity Polarity>
LEDDisplay<Polarity>::LEDDisplay(uint8_t ledOnePin, uint8_t ledTwoPin, uint8_t ledThreePin)
  : ledOne(ledOnePin), ledTwo(ledTwoPin), ledThree(ledThreePin) {
  pinMode(ledOne, OUTPUT);
  pinMode(ledTwo, OUTPUT);
  pinMode(ledThree, OUTPUT);

  ledTurnOnMask(allLedOffMask);

  errorData.parent = this;
  errorData.errorLedsPins = { ledThree, ledTwo, ledOne };

  // Create FreeRTOS timer (auto-reload disabled; we’ll restart it manually)
  errorData.blinkTimer = xTimerCreate(
    "led_blink",
    ErrorStruct::LONG_BLINK_DELAY,
    pdFALSE,     // one-shot, we control restarts
    &errorData,  // timer ID (passed to callback)
    &ErrorStruct::blinkCallback);
}

template<ActivePolarity Polarity>
void LEDDisplay<Polarity>::writeLed(uint8_t pin, LedState ledState) const {
  if constexpr (Polarity == ActivePolarity::activeHigh)
    digitalWrite(pin, ledState == LedState::ledOn ? HIGH : LOW);
  else if constexpr (Polarity == ActivePolarity::activeLow)
    digitalWrite(pin, ledState == LedState::ledOn ? LOW : HIGH);
}

template<ActivePolarity Polarity>
void LEDDisplay<Polarity>::ledTurnOnMask(uint8_t mask) {
  writeLed(ledOne, (mask & ledOneMask)
                     ? LedState::ledOn
                     : LedState::ledOff);
  writeLed(ledTwo, (mask & ledTwoMask)
                     ? LedState::ledOn
                     : LedState::ledOff);
  writeLed(ledThree, (mask & ledThreeMask)
                       ? LedState::ledOn
                       : LedState::ledOff);
}

// ===== Public API =====
template<ActivePolarity Polarity>
void LEDDisplay<Polarity>::showMode(LEDDisplay<Polarity>::Mode mode, bool saving) {
  showError(-1);

  uint8_t mask = ModeMasks[static_cast<size_t>(mode)];
  if (saving) {
    if (mode == Mode::Cool) mask = MaskCoolSave;
    if (mode == Mode::Heat) mask = MaskHeatSave;
  }

  ledTurnOnMask(mask);
}

template<ActivePolarity Polarity>
void LEDDisplay<Polarity>::showError(int errorNo) {
  ledTurnOnMask(allLedOffMask);
  if (xTimerIsTimerActive(errorData.blinkTimer))
    xTimerStop(errorData.blinkTimer, 0);

  if (errorNo < 0 && errorData.totalBlinks > 0) {
    errorData.totalBlinks = 0;
    errorData.blinkCount = 0;
    errorData.errorLedState = LedState::ledOff;
    return;
  }

  errorData.totalBlinks = (errorNo % 10) + 1;
  errorData.blinkCount = 0;
  errorData.errorLedState = LedState::ledOff;

  // to avoid overflow
  uint8_t idx = errorNo / 10;
  if (idx >= errorData.errorLedsPins.size()) idx = 0;
  errorData.errorLed = errorData.errorLedsPins[idx];

  xTimerChangePeriod(errorData.blinkTimer, ErrorStruct::LONG_BLINK_DELAY, 0);
}

template<ActivePolarity Polarity>
void LEDDisplay<Polarity>::ErrorStruct::blinkCallback(TimerHandle_t xTimer) {
  auto* self = static_cast<ErrorStruct*>(pvTimerGetTimerID(xTimer));
  self->handleBlink();
}

template<ActivePolarity Polarity>
void LEDDisplay<Polarity>::ErrorStruct::handleBlink() {
  if (totalBlinks == 0) return;  // no error active

  errorLedState = (errorLedState == LedState::ledOn)
                    ? LedState::ledOff
                    : LedState::ledOn;

  parent->writeLed(errorLed,
                   errorLedState);

  if (errorLedState == LedState::ledOff) {
    blinkCount++;

    if (blinkCount >= totalBlinks) {
      blinkCount = 0;
      xTimerChangePeriod(blinkTimer, LONG_BLINK_DELAY, 0);

      return;
    }
  }

  xTimerChangePeriod(blinkTimer, NORMAL_BLINK_DELAY, 0);
}

template<ActivePolarity Polarity>
LEDDisplay<Polarity>::~LEDDisplay() {
  if (errorData.blinkTimer) {
    xTimerStop(errorData.blinkTimer, 0);
    xTimerDelete(errorData.blinkTimer, 0);
    errorData.blinkTimer = nullptr;
  }
}

template<ActivePolarity Polarity>
void LEDDisplay<Polarity>::enable() {
}

template<ActivePolarity Polarity>
void LEDDisplay<Polarity>::disable() {
}

#endif