#ifndef SEVEN_SEGMENT_DISPLAY_H
#define SEVEN_SEGMENT_DISPLAY_H

#include <cstdint>
#include <array>
#include <TM1637Display.h>

class SevenSegmentDisplay {
public:
  enum class Type {
    OneTon,
    Cassette
  };

  enum class Mode {
    Cool,
    Heat,
    Dry,
    Fan,
    AutoCool,
    AutoHeat,
  };

  enum class Letters {
    E,
    F,
    H,
    Blank
  };

  enum class Digits {
    Zero,
    One,
    Two,
    Three,
    Four,
    Five,
    Six,
    Seven,
    Eight,
    Nine,
    Blank
  };

  SevenSegmentDisplay(int clkPin, int dioPin, Type t)
    : type(t), driver(clkPin, dioPin, bitDelay) {
    is_enabled = false;
  }

  void showTemperature(int temp);
  void showMode(Mode mode, bool saving);
  void showError(uint8_t error_no);
  void enable();
  void disable();

private:
  struct LedPatternSet {
    uint8_t cool, heat, dry, eco_cool, eco_heat, all_off;
  };

  static constexpr std::array<LedPatternSet, 2> ledPatterns{ {
    { 0x20, 0x01, 0x08, 0x22, 0x03, 0x00 },  // OneTon
    { 0x01, 0x08, 0x02, 0x05, 0x0C, 0x00 }   // Cassette
  } };

  static uint8_t encodeLetter(Letters l);
  static uint8_t encodeDigit(Digits d);

  void ledOff();

  static constexpr int  minimumBrightness = 0;
  static constexpr int maximumBrightness = 7;
  static constexpr int bitDelay = 10;
  bool is_enabled;
  
  Type type;
  TM1637Display driver;
};

#endif