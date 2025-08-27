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

  void showMode(Mode mode, bool saving);
  void showError(int errorNo);
  void enable();
  void disable();

private:
  struct ErrorPattern {
    int errorNo;
    uint8_t blinks;
    uint8_t ledPin;
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

  static constexpr uint32_t blinkDelay = 500;
  static constexpr uint32_t errorDelay = 1000;

  uint8_t ledOne;
  uint8_t ledTwo;
  uint8_t ledThree;
  std::array<ErrorPattern, 20> errorPatterns;

  void turnOnMask(uint8_t mask);
  inline void writeLed(uint8_t pin, LedState ledState) const;
};

template<ActivePolarity Polarity>
LEDDisplay<Polarity>::LEDDisplay(uint8_t ledOnePin, uint8_t ledTwoPin, uint8_t ledThreePin)
  : ledOne(ledOnePin), ledTwo(ledTwoPin), ledThree(ledThreePin),

    errorPatterns{ { { 0, 1, ledThreePin }, { 1, 2, ledThreePin }, { 2, 3, ledThreePin }, { 3, 4, ledThreePin }, { 4, 5, ledThreePin }, { 5, 6, ledThreePin }, { 10, 1, ledTwoPin }, { 11, 2, ledTwoPin }, { 12, 3, ledTwoPin }, { 13, 4, ledTwoPin }, { 14, 5, ledTwoPin }, { 15, 6, ledTwoPin }, { 16, 1, ledOnePin }, { 20, 7, ledOnePin }, { 21, 2, ledOnePin }, { 22, 6, ledOnePin }, { 23, 4, ledOnePin }, { 24, 5, ledOnePin }, { 25, 3, ledOnePin }, { 29, 8, ledOnePin } } } {

  pinMode(ledOne, OUTPUT);
  pinMode(ledTwo, OUTPUT);
  pinMode(ledThree, OUTPUT);

  turnOnMask(allLedOffMask);
}

template<ActivePolarity Polarity>
void LEDDisplay<Polarity>::writeLed(uint8_t pin, LedState ledState) const {
  if constexpr (Polarity == ActivePolarity::activeHigh)
    digitalWrite(pin, ledState == LedState::ledOn ? HIGH : LOW);
  else if constexpr (Polarity == ActivePolarity::activeLow)
    digitalWrite(pin, ledState == LedState::ledOn ? LOW : HIGH);
}

template<ActivePolarity Polarity>
void LEDDisplay<Polarity>::turnOnMask(uint8_t mask) {
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

  uint8_t mask = ModeMasks[static_cast<size_t>(mode)];
  if (saving) {
    if (mode == Mode::Cool) mask = MaskCoolSave;
    if (mode == Mode::Heat) mask = MaskHeatSave;
  }

  turnOnMask(mask);
}

template<ActivePolarity Polarity>
void LEDDisplay<Polarity>::showError(int errorNo) {
  for (auto& p : errorPatterns) {
    if (p.errorNo == errorNo) {
      for (int i = 0; i < p.blinks; i++) {
        writeLed(p.ledPin, LedState::ledOn);
        delay(blinkDelay);
        writeLed(p.ledPin, LedState::ledOff);
        delay(blinkDelay);
      }

      delay(errorDelay);
      return;
    }
  }
}

template<ActivePolarity Polarity>
void LEDDisplay<Polarity>::enable() {
}

template<ActivePolarity Polarity>
void LEDDisplay<Polarity>::disable() {
}

#endif