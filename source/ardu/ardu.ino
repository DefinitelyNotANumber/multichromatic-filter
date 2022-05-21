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

#define MODE_SWITCH 10     // RGB alpha/CMYK switch
#define COL_NEG_SWITCH 2  // color negate switch
#define NEG_LED 3         // LED indicating if the color is negated or not
#define OUT_SWITCH 4      // output enable switch
#define OUT_LED 5         // output enable LED indicator

#define POT_C A7  // potentiometer used to set cyan/red value (depending on the state of MODE_SWITCH)
#define POT_M A1  // magenta/green
#define POT_Y A2  // yellow/blue
#define POT_K A3  // key/alpha

float C = 0;  // cyan
float M = 0;  // magenta
float Y = 0;  // yellow
float K = 0;  // key

unsigned char R = 0;  // red
unsigned char G = 0;  // green
unsigned char B = 0;  // blue

Arduino_DataBus *bus = new Arduino_HWSPI(TFT_DC, TFT_CS);
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
    C = analogRead(POT_C) * (255.0 / 1023.0);
    M = analogRead(POT_M) * (255.0 / 1023.0);
    Y = analogRead(POT_Y) * (255.0 / 1023.0);
    K = analogRead(POT_K) * (255.0 / 1023.0);  // ^ setting CMYK values
    
    if (M > Y) R = M - Y + K;
    else R = Y - M + K;
    if (C > Y) G = C - Y + K;
    else G = Y - C + K;
    if (C > M) B = C - M + K;
    else B = M - C + K;
  }
  else {
    a = 255 - analogRead(POT_K) * (255.0 / 1023.0);
    R = analogRead(POT_C) * (255.0 / 1023.0) - a / 1023;
    G = analogRead(POT_M) * (255.0 / 1023.0) - a / 1023;
    B = analogRead(POT_Y) * (255.0 / 1023.0) - a / 1023;  // ^ setting RGB alpha values directly
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
    lcd.print(255 - C, 0);
    lcd.print(", ");
    lcd.setCursor(7, 0);
    lcd.print("M ");
    lcd.print(255 - M, 0);
    lcd.setCursor(0, 1);
    lcd.print("Y ");
    lcd.print(255 - Y, 0);
    lcd.print(", ");
    lcd.setCursor(7, 1);
    lcd.print("K ");
    lcd.print(255 - K, 0); // ^ displaying CMYK values
  }
  else {
    lcd.setCursor(0, 0);
    lcd.print("R ");
    lcd.print(255 - analogRead(POT_C) * (255.0 / 1023.0), 0);
    lcd.print(", ");
    lcd.setCursor(7, 0);
    lcd.print("G ");
    lcd.print(255 - analogRead(POT_M) * (255.0 / 1023.0), 0);
    lcd.setCursor(0, 1);
    lcd.print("B ");
    lcd.print(255 - analogRead(POT_Y) * (255.0 / 1023.0), 0);
    lcd.print(", ");
    lcd.setCursor(7, 1);
    lcd.print("a ");
    lcd.print(100 - analogRead(POT_K) * (100.0 / 1023.0), 0);
    lcd.print("%");  // ^ displaying RGB alpha values
  }

  if (digitalRead(OUT_SWITCH) == LOW) {
    gfx->fillScreen(gfx->color565(255 - R, 255 - G, 255 - B));  // writing set RGB values to the main LCD
    digitalWrite(OUT_LED, HIGH);
  }
  else {
    gfx->fillScreen(WHITE);  // clearing out the main LCD if output enable switch is off
    digitalWrite(OUT_LED, LOW);
  }
  delay(200);
}
