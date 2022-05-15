/*

Copyright 2022 Not_A_Number

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include <Arduino_GFX_Library.h>  // LCD control library for the modified SPI display
#include <LiquidCrystal_I2C.h>    // a library for controlling the info LCD

#define TFT_CS 9
#define TFT_DC 8
#define TFT_RST 7
#define TFT_BL 6   // ^ additional pins for the LCD

#define MODE_SWITCH 1     // RGB alpha/CMYK switch
#define COL_NEG_SWITCH 2  // color negate switch
#define NEG_LED 3         // LED indicating if the color is negated or not
#define OUT_SWITCH 4      // output enable switch
#define OUT_LED 5         // output enable LED indicator

#define POT_C A0  // potentiometer used to set cyan/red value (depending on the state of MODE_SWITCH)
#define POT_M A1  // magenta/green
#define POT_Y A2  // yellow/blue
#define POT_K A3  // key/alpha

unsigned char C = 0;  // cyan
unsigned char M = 0;  // magenta
unsigned char Y = 0;  // yellow
unsigned char K = 0;  // key

unsigned char R = 0;  // red
unsigned char G = 0;  // green
unsigned char B = 0;  // blue

const int RANGE = 255 / 1023;

Arduino_DataBus *bus = new Arduino_NRFXSPI(TFT_DC, TFT_CS, 13 /* SCK */, 11 /* MOSI */, 12 /* MISO */);
Arduino_GFX *gfx = new Arduino_SSD1283A(bus, TFT_RST, 0 /* rotation */); // ^ bus and gfx pinout config

LiquidCrystal_I2C lcd(0x27, 16, 2);  // I2C address 0x27, 16 columns and 2 rows

int a = 0;  // alpha


void showStartupScreen() {
  for (int i = 0; i <= 10; ++i) {  // startup info, blinking for two seconds
    if (i % 2 == 0) {
      lcd.clear();
      lcd.home();
      lcd.print("Starting up..");
    }
    else {
      lcd.print(".");
    }
    delay(200);
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(MODE_SWITCH, INPUT_PULLUP);
  pinMode(COL_NEG_SWITCH, INPUT_PULLUP);
  pinMode(OUT_SWITCH, INPUT_PULLUP);
  pinMode(NEG_LED, OUTPUT);
  pinMode(OUT_LED, OUTPUT);

  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  gfx->begin();
  lcd.init();           // initialize the info LCD
  lcd.backlight();      // light up the backlight
  lcd.home();           // set LCD cursor to home position
  showStartupScreen();
  lcd.clear();
  lcd.noCursor();       // turning off the cursor
}


void loop() {
  if (digitalRead(MODE_SWITCH) == LOW) {
    C = analogRead(POT_C) * RANGE;
    M = analogRead(POT_M) * RANGE;
    Y = analogRead(POT_Y) * RANGE;
    K = analogRead(POT_K) * RANGE;  // ^ setting CMYK values
    
    if (M > Y) R = M - Y + K;
    else R = Y - M + K;
    if (C > Y) G = C - Y + K;
    else G = Y - C + K;
    if (C > M) B = C - M + K;
    else B = M - C + K;
  }
  else {
    a = 255 - analogRead(POT_K) * RANGE;
    if (a == 0) auto RANGE_A = 0;
    else auto RANGE_A = RANGE - a / 1023;
    R = analogRead(POT_C) * RANGE_A;
    G = analogRead(POT_M) * RANGE_A;
    B = analogRead(POT_Y) * RANGE_A;  // ^ setting RGB alpha values directly
  }

  if (digitalRead(COL_NEG_SWITCH) == LOW) { // setting the output color to a negative of what's set
    a = 255 - a;
    R = 255 - a - R;
    G = 255 - a - G;
    B = 255 - a - B;
    digitalWrite(NEG_LED, HIGH); // indicating that color negating is on
  }
  else digitalWrite(NEG_LED, LOW);

  lcd.clear();
  lcd.home();
  
  if (digitalRead(MODE_SWITCH) == LOW) {
    lcd.setCursor(0, 0);
    lcd.print("C ");
    lcd.print(C);
    lcd.print(", ");
    lcd.setCursor(7, 0);
    lcd.print("M ");
    lcd.print(M);
    lcd.setCursor(0, 2);
    lcd.print("Y ");
    lcd.print(Y);
    lcd.print(", ");
    lcd.setCursor(7, 2);
    lcd.print("K ");
    lcd.print(K); // ^ displaying CMYK values
  }
  else {
    lcd.setCursor(0, 0);
    lcd.print("R ");
    lcd.print(analogRead(POT_C) * RANGE);
    lcd.print(", ");
    lcd.setCursor(7, 0);
    lcd.print("G ");
    lcd.print(analogRead(POT_M) * RANGE);
    lcd.setCursor(0, 2);
    lcd.print("B ");
    lcd.print(analogRead(POT_Y) * RANGE);
    lcd.print(", ");
    lcd.setCursor(7, 2);
    lcd.print("a ");
    lcd.print(100 - analogRead(POT_K) * (100 / 1023));
    lcd.print("%");  // ^ displaying RGB alpha values
  }

  if (digitalRead(OUT_SWITCH) == LOW) {
    gfx->fillScreen(gfx->color565(R, G, B));  // writing set RGB values to the main LCD
    digitalWrite(OUT_LED, HIGH);
  }
  else {
    gfx->fillScreen(BLACK);  // clearing out the main LCD if output enable switch is off
    digitalWrite(OUT_LED, LOW);
  }
}
