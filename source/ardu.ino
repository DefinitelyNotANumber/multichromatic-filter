/*Copyright 2022 Not_A_Number

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.*/



#include <Arduino_GFX_Library.h> // LCD control library for the modified SPI display
#include <LiquidCrystal_I2C.h> // a library for controlling the info LCD

#define TFT_CS 9
#define TFT_DC 8
#define TFT_RST 7
#define TFT_BL 6 // ^ additional pins for the LCD

Arduino_DataBus *bus = new Arduino_NRFXSPI(TFT_DC, TFT_CS, 13 /* SCK */, 11 /* MOSI */, 12 /* MISO */);
Arduino_GFX *gfx = new Arduino_SSD1283A(bus, TFT_RST, 0 /* rotation */); // ^ bus and gfx pinout config

LiquidCrystal_I2C lcd(0x27, 16, 2); // I2C address 0x27, 16 columns and 2 rows

#define modeSwitch 1 // RGBalpha/CMYK switch
#define colNegSwitch 2 // color negate switch
#define negLED 3 // LED indicating if the color is negated or not
#define outSwitch 4 // output enable switch
#define outLED 5 // output enable LED indicator

#define potC A0 // potentiometer used to set cyan/red value (depending on the state of modeSwitch)
#define potM A1 // magenta/green
#define potY A2 // yellow/blue
#define potK A3 // key/alpha

int C = 0; // cyan
int M = 0; // magenta
int Y = 0; // yellow
int K = 0; // key

int R = 0; // red
int G = 0; // green
int B = 0; // blue
int a = 0; // alpha

int32_t w, h, n, n1, cx, cy, cx1, cy1, cn, cn1;
uint8_t tsa, tsb, tsc, ds;

void setup() {
  Serial.begin(9600);
  gfx->begin();
  lcd.init(); // initialize the info LCD
  lcd.backlight(); // light up the backlight 
  lcd.home(); // set LCD cursor to home position
  for (int i = 0; i <= 10; i++) { // startup info, blinking for two seconds
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
  lcd.clear();
  lcd.noCursor(); // turning off the cursor

}

void loop() {
  if (digitalRead(modeSwitch) == LOW){
    int sensorValue = analogRead(potC);
    C = sensorValue * (255 / 1023);
    sensorValue = analogRead(potM);
    M = sensorValue * (255 / 1023);
    sensorValue = analogRead(potY);
    Y = sensorValue * (255 / 1023);
    sensorValue = analogRead(potK);
    K = sensorValue * (255 / 1023); // ^ setting CMYK values

    R = M - Y;
    if (R < 0) {
      R = R * (-1);
    }
    R = R + K;
    if (R > 255) {
      R = 255;
    }
    
    G = C - Y;
    if (G < 0) {
      G = G * (-1);
    }
    G = G + K;
    if (G > 255) {
      G = 255;
    }
    
    B = C - M;
    if (B < 0) {
      B = B * (-1);
    }
    B = B + K;
    if (B > 255) {
      B = 255;
    }         // ^ converting CMYK to RGB
  }
  else {
    int sensorValue = analogRead(potK);
    a = 255 - sensorValue * (255 / 1023);
    if (a == 0) {
      a++;
    }
    sensorValue = analogRead(potC);
    R = sensorValue * (255 / 1023) / a;
    sensorValue = analogRead(potM);
    G = sensorValue * (255 / 1023) / a;
    sensorValue = analogRead(potY);
    B = sensorValue * (255 / 1023) / a;  // ^ setting RGBalpha values directly
  }
  if (digitalRead(colNegSwitch) == LOW) { // setting the output color to a negative of what's set
    R = 255 / a - R;
    G = 255 / a - G;
    B = 255 / a - B;
    digitalWrite(negLED, HIGH); // indicating that color negating is on
  }
  else {
    digitalWrite(negLED, LOW);
  }
  lcd.clear();
  lcd.home();
  if (digitalRead(modeSwitch) == LOW) {
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
    lcd.print(analogRead(potC) * (255 / 1023));
    lcd.print(", ");
    lcd.setCursor(7, 0);
    lcd.print("G ");
    lcd.print(analogRead(potM) * (255 / 1023));
    lcd.setCursor(0, 2);
    lcd.print("B ");
    lcd.print(analogRead(potY) * (255 / 1023));
    lcd.print(", ");
    lcd.setCursor(7, 2);
    lcd.print("a ");
    lcd.print(100 - analogRead(potK) * (100 / 1023));
    lcd.print("%"); // ^ displaying RGBaplha values
  }
  if (digitalRead(outSwitch) == LOW) {
    gfx->fillScreen(gfx->color565(R, G, B)); // writing set RGB values to the main LCD
    digitalWrite(outLED, HIGH);
  }
  else {
    gfx->fillScreen(BLACK); // clearing out the main LCD if output enable switch is off
    digitalWrite(outLED, LOW);
  }
}
