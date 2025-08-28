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

    static const uint32_t NORMAL_BLINK_DELAY = 500000;
    static const uint32_t LONG_BLINK_DELAY = 1000000;
    esp_timer_handle_t blinkTimer = nullptr;
    std::array<uint8_t, 3> errorLedsPins;
    uint8_t totalBlinks = 0;
    uint8_t blinkCount = 0;
    LedState errorLedState = LedState::ledOff;
    uint8_t errorLed = 0;

    LEDDisplay<Polarity>* parent = nullptr;

    static void blinkCallback(void* arg);
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

  esp_timer_create_args_t timer_args = {};
  timer_args.callback = &ErrorStruct::blinkCallback;
  timer_args.arg = &errorData;
  timer_args.dispatch_method = ESP_TIMER_TASK;
  timer_args.name = "led_blink";

  esp_timer_create(&timer_args, &errorData.blinkTimer);
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
  if (esp_timer_is_active(errorData.blinkTimer)) esp_timer_stop(errorData.blinkTimer);

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

  esp_timer_start_once(errorData.blinkTimer, ErrorStruct::LONG_BLINK_DELAY);
}

template<ActivePolarity Polarity>
void LEDDisplay<Polarity>::ErrorStruct::blinkCallback(void* arg) {
  auto* self = static_cast<ErrorStruct*>(arg);
  
  Serial.println("Calling handle blink");
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

  if (blinkCount == 0 && errorLedState == LedState::ledOn) {
    esp_timer_stop(blinkTimer);
    esp_timer_start_periodic(blinkTimer, ErrorStruct::NORMAL_BLINK_DELAY);
    return;
  }

  if (errorLedState == LedState::ledOff) {
    blinkCount++;

    if (blinkCount >= totalBlinks) {
      blinkCount = 0;
      esp_timer_stop(blinkTimer);
      esp_timer_start_once(blinkTimer, ErrorStruct::LONG_BLINK_DELAY);
    }
  }
}

template<ActivePolarity Polarity>
LEDDisplay<Polarity>::~LEDDisplay() {
  if (errorData.blinkTimer) {
    esp_timer_stop(errorData.blinkTimer);    // ensure stopped
    esp_timer_delete(errorData.blinkTimer);  // free resources
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