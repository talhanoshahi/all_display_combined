#include "display.h"

// Display <SevenSegmentDisplay > seg3(1, 2, SevenSegmentDisplay::Type::OneTon);

// void setup() {
//   Serial.begin(912600);
// }

// void loop() {
//   seg3.enable ();
//   seg3.showTemperature(20);
//   seg3.showMode(1, 0);

//   delay (3000);

//   seg3.showError(11);

//   delay (3000);
//   seg3.showMode(1, 0);
//   delay (3000);
//   seg3.disable ();
//   delay (3000);
//   seg3.showTemperature(25);
//   delay (3000);

// }

// Display <SevenSegmentDisplay > seg4(10, 11, SevenSegmentDisplay::Type::Cassette);

// void setup() {
//   Serial.begin(912600);
//   // put your setup code here, to run once:
//   seg4.showTemperature(21);
//   seg4.showMode(2, 0);

//   delay (3000);

//   seg4.showError(3);
// }

// void loop() {
//   // put your main code here, to run repeatedly:

// }


// Display<LCDDisplay > lcd(3, 2, 1, 4);

// void setup() {
//   Serial.begin(912600);
// }

// void loop() {
//   lcd.enable();
//   lcd.showTemperature(19);
//   lcd.showMode(1, 0);

//   delay(3000);

//   lcd.showError(3);

//   delay(3000);
//   lcd.disable();

//   delay(2000);
// }


Display<LEDDisplay<ActivePolarity::activeLow>> led(1, 2, 3);

void setup() {
  Serial.begin(912600);
  led.showError(13);
}

void loop() {
  // led.enable();
  // led.showTemperature(19);
  // led.showMode(1, 0);

  // delay(3000);

  // led.showError(13);

  // delay(3000);
  // led.disable();

  // delay(2000);
}