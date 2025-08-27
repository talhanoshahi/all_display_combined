#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H

#include <HT1621.h>

class LCDDisplay {

public:
  enum class SigChar1 {
    C,
    H,
    A,
    D,
    F,
    E,
    One,
    Two,
    Three
  };

  enum class SigChar2 {
    Zero,
    One,
    Two,
    Three,
    Four,
    Five,
    Six,
    Seven,
    Eight,
    Nine
  };


  LCDDisplay(int CSPin, int RWPin, int dataPin, int backlightPin)
    : CSPin(CSPin), RWPin(RWPin), dataPin(dataPin), backlightPin(backlightPin),
      driver(CSPin, RWPin, dataPin),
      is_enabled(false) {
    pinMode(backlightPin, OUTPUT);
    pinMode(CSPin, OUTPUT);
    pinMode(RWPin, OUTPUT);
    pinMode(dataPin, OUTPUT);

    driver.begin();

    driver.sendCommand(HT1621::SYS_EN);
    driver.sendCommand(HT1621::RC256K);
    driver.sendCommand(HT1621::BIAS_THIRD_4_COM);
  }

  void showError(uint8_t error_no);
  void showTemperature(int temp);
  void enable();
  void disable();

private:
  int CSPin;
  int RWPin;
  int dataPin;
  int backlightPin;
  HT1621 driver;
  bool is_enabled;

  static constexpr int tempCelsiusRegisterAddress = 4;
  static constexpr int tempCelsiusEnableMask = 0x8;
  static constexpr int tempCelsiusClearMask = 0x0;
  static constexpr uint8_t sig1Addr[3] = { 0, 1, 2 };
  static constexpr uint8_t sig2Addr[3] = { 3, 8, 9 };

  struct Glyph {
    uint8_t b0;
    uint8_t b1;
    uint8_t b2;
  };

  void tempCelciusSet(bool enabled);
  void writeOneGlyph(const Glyph& g, const uint8_t addr[3]);
  void writeGlyphs(const SigChar1 ch1, const SigChar2 ch2);
  void backlightOn(bool on);

  // Sig1 glyphs (letters + a few numbers)
  static constexpr std::array<Glyph, 9> sig1Glyphs{ {
    { 0x02, 0x0E, 0x00 },  // C
    { 0x0C, 0x0C, 0x02 },  // H
    { 0x0E, 0x0C, 0x02 },  // A
    { 0x0E, 0x0E, 0x00 },  // D
    { 0x02, 0x0C, 0x02 },  // F
    { 0x02, 0x0E, 0x02 },  // E
    { 0x0C, 0x00, 0x00 },  // 1
    { 0x06, 0x06, 0x02 },  // 2
    { 0x0E, 0x02, 0x02 }   // 3
  } };

  // Sig2 glyphs (digits 0–9)
  static constexpr std::array<Glyph, 10> sig2Glyphs{ {
    { 0x0E, 0x00, 0x0E },  // 0
    { 0x0C, 0x00, 0x00 },  // 1
    { 0x06, 0x02, 0x06 },  // 2
    { 0x0E, 0x02, 0x02 },  // 3
    { 0x0C, 0x02, 0x08 },  // 4
    { 0x0A, 0x02, 0x0A },  // 5
    { 0x0A, 0x02, 0x0E },  // 6
    { 0x0E, 0x00, 0x00 },  // 7
    { 0x0E, 0x02, 0x0E },  // 8
    { 0x0E, 0x02, 0x08 }   // 9
  } };
};

#endif