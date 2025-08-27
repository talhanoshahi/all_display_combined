#include "seven_segment_display.h"

void SevenSegmentDisplay::showTemperature(int temp) {
  if (!is_enabled) return;
  
  driver.showNumberDec(temp, true, 2, 0);
}

// ==== Mode LEDs ====
void SevenSegmentDisplay::showMode(SevenSegmentDisplay::Mode mode, bool saving) {
  if (!is_enabled) return;
  
  uint8_t ledPattern = ledPatterns[(int)type].all_off;

  switch (mode) {
    case Mode::Cool:
      ledPattern = saving ? ledPatterns[(int)type].eco_cool
                          : ledPatterns[(int)type].cool;
      break;
    case Mode::Heat:
      ledPattern = saving ? ledPatterns[(int)type].eco_heat
                          : ledPatterns[(int)type].heat;
      break;
    case Mode::Dry: ledPattern = ledPatterns[(int)type].dry; break;
    case Mode::Fan: ledPattern = ledPatterns[(int)type].all_off; break;
    case Mode::AutoCool: ledPattern = ledPatterns[(int)type].cool; break;
    case Mode::AutoHeat: ledPattern = ledPatterns[(int)type].heat; break;
  }
  driver.setSegments(&ledPattern, 1, 2);
}

uint8_t SevenSegmentDisplay::encodeLetter(SevenSegmentDisplay::Letters l) {
  switch (l) {
    case Letters::E: return 0x79;
    case Letters::F: return 0x71;
    case Letters::H: return 0x76;
    case Letters::Blank: return 0x00;
  }
  return 0x00;
}

uint8_t SevenSegmentDisplay::encodeDigit(SevenSegmentDisplay::Digits d) {
  switch (d) {
    case Digits::Zero: return TM1637Display::encodeDigit(0);
    case Digits::One: return TM1637Display::encodeDigit(1);
    case Digits::Two: return TM1637Display::encodeDigit(2);
    case Digits::Three: return TM1637Display::encodeDigit(3);
    case Digits::Four: return TM1637Display::encodeDigit(4);
    case Digits::Five: return TM1637Display::encodeDigit(5);
    case Digits::Six: return TM1637Display::encodeDigit(6);
    case Digits::Seven: return TM1637Display::encodeDigit(7);
    case Digits::Eight: return TM1637Display::encodeDigit(8);
    case Digits::Nine: return TM1637Display::encodeDigit(9);
    case Digits::Blank: return 0x00;
  }
  return 0x00;
}

void SevenSegmentDisplay::ledOff() {
  uint8_t ledPattern = ledPatterns[(int)type].all_off;
  driver.setSegments(&ledPattern, 1, 2);
}

void SevenSegmentDisplay::showError(uint8_t error_no) {
  if (!is_enabled) return;

  ledOff();

  uint8_t error_letter = error_no / 10;
  uint8_t error_digit = (error_no % 10);

  uint8_t segs[2] = { encodeLetter(static_cast<Letters>(error_letter)), encodeDigit(static_cast<Digits>(error_digit)) };
  driver.setSegments(segs, 2, 0);
}

void SevenSegmentDisplay::disable() {
  is_enabled = false;

  driver.setBrightness(minimumBrightness, is_enabled);
  driver.clear();
}

void SevenSegmentDisplay::enable() {
  is_enabled = true;
  driver.setBrightness(maximumBrightness, is_enabled);
}
