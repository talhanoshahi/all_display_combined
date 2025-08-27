#ifndef DISPLAY_H
#define DISPLAY_H

#include "lcd_display.h"
#include "led_display.h"
#include "seven_segment_display.h"

template<typename Display>
struct DisplayTraits {
  static constexpr bool hasTemperature = false;
  static constexpr bool hasMode = false;
  static constexpr bool hasError = false;
};

template<>
struct DisplayTraits<SevenSegmentDisplay> {
  static constexpr bool hasTemperature = true;
  static constexpr bool hasMode = true;
  static constexpr bool hasError = true;  // if SevenSegmentDisplay doesn't support errors
};

template<>
struct DisplayTraits<LCDDisplay> {
  static constexpr bool hasTemperature = true;
  static constexpr bool hasMode = false;
  static constexpr bool hasError = true;
};

template<ActivePolarity Polarity>
struct DisplayTraits<LEDDisplay<Polarity>> {
  static constexpr bool hasTemperature = false;
  static constexpr bool hasMode = true;
  static constexpr bool hasError = true;
};

template<typename DisplayType>
class Display {
public:
  DisplayType display;

  template<typename... Args>
  Display(Args&&... args)
    : display(std::forward<Args>(args)...) {}

  inline void showTemperature(int temp) {
    if constexpr (DisplayTraits<DisplayType>::hasTemperature) {
      display.showTemperature(temp);
    }
  }

  inline void showMode(int mode, bool savingMode) {
    if constexpr (DisplayTraits<DisplayType>::hasMode) {
      typename DisplayType::Mode mode_enum = static_cast<DisplayType::Mode>(mode);
      display.showMode(mode_enum, savingMode);
    }
  }

  inline void showError(int error_no) {
    if constexpr (DisplayTraits<DisplayType>::hasError) {
      display.showError(error_no);
    }
  }

  inline void enable() {
    display.enable();
  }

  inline void disable() {
    display.disable();
  }
};

#endif