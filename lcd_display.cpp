#include "lcd_display.h"

void LCDDisplay::writeOneGlyph(const LCDDisplay::Glyph& g, const uint8_t addr[3]) {
  driver.write(addr[0], g.b0);
  driver.write(addr[1], g.b1);
  driver.write(addr[2], g.b2);
}

void LCDDisplay::tempCelciusSet(bool enabled) {
  if (enabled) {
    driver.write(tempCelsiusRegisterAddress, tempCelsiusEnableMask);
    return;
  }

  driver.write(tempCelsiusRegisterAddress, tempCelsiusClearMask);
}

void LCDDisplay::backlightOn(bool on) {
  digitalWrite(backlightPin, on ? HIGH : LOW);
}

void LCDDisplay::disable() {
  backlightOn(false);

  for (uint8_t addr = 0; addr < 32; addr++) {
    driver.write(addr, 0x00);
  };

  driver.sendCommand(HT1621::LCD_OFF);

  is_enabled = false;
}

void LCDDisplay::enable() {

  driver.sendCommand(HT1621::LCD_ON);

  backlightOn(true);
  is_enabled = true;
}

void LCDDisplay::writeGlyphs(const LCDDisplay::SigChar1 ch1, const LCDDisplay::SigChar2 ch2) {
  const Glyph& g1 = sig1Glyphs[static_cast<int>(ch1)];
  const Glyph& g2 = sig2Glyphs[static_cast<int>(ch2)];

  writeOneGlyph(g1, sig1Addr);
  writeOneGlyph(g2, sig2Addr);
}

void LCDDisplay::showError(uint8_t error_no) {
  if (!is_enabled) return;

  uint8_t error_letter = error_no / 10;
  uint8_t error_digit = error_no % 10;

  SigChar1 letter;
  switch (error_letter) {
    case 0: letter = SigChar1::E; break;
    case 1: letter = SigChar1::F; break;
    case 2: letter = SigChar1::H; break;
    default: letter = SigChar1::C; break;  // fallback
  }

  SigChar2 digit = static_cast<SigChar2>(error_digit);

  // Use your existing glyph writer
  writeGlyphs(letter, digit);

  tempCelciusSet(false);
}

void LCDDisplay::showTemperature(int temp) {
  if (!is_enabled) return;

  uint8_t tenth_digit = temp / 10;
  uint8_t oneth_digit = temp % 10;

  SigChar1 digit1;
  switch (tenth_digit) {
    case 1: digit1 = SigChar1::One; break;
    case 2: digit1 = SigChar1::Two; break;
    case 3: digit1 = SigChar1::Three; break;
    default: digit1 = SigChar1::One; break;  // fallback safe glyph
  }

  SigChar2 digit2 = static_cast<SigChar2>(oneth_digit);

  writeGlyphs(digit1, digit2);

  tempCelciusSet(true);
}