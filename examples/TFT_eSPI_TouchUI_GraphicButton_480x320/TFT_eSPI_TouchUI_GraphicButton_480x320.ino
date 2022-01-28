/*
   Example of drawing a "slider controller" and using
   the touch screen to change it's state (for 480x320 px).

   This libary introduce sevaral knob styles
   Based on the popular Bodmer's eSPI TFT Libary.

   Touch handling for XPT2046 based screens is handled by
   the TFT_eSPI library.

   Calibration data is stored in SPIFFS so we need to include it

   *** Created by JLGOASIS 1/02/22 ***
*/

// Calibration data is stored in SPIFFS so we need to include it
#include "FS.h"
#include <SPI.h>
#include <TFT_eSPI.h> // Hardware-specific library

#include <TFT_eSPI_TouchUI.h>

// Invoke custom library
TFT_eSPI tft = TFT_eSPI();

#include "icons.h"

#define CALIBRATION_FILE "/TouchCalData0"
#define REPEAT_CAL false

#define TFT_GREY 0x4208

TFT_eSPI_TouchUI button[4];

bool power, mute = true;
uint8_t level = 5;

void setup(void) {
  Serial.begin(115200);
  Serial.println();

  tft.init();
  tft.setRotation(0);

  // call screen calibration
  touch_calibrate();

  tft.fillScreen(TFT_BLACK);

  tft.fillRect(0, 0, tft.width(), 20, TFT_BLUE);
  tft.setTextColor(TFT_WHITE, TFT_BLUE);
  tft.drawCentreString("GRAPHIC BUTTON TEST", tft.width() / 2, 2, 2);

  // Init parameters:
  // &tft, x, y , w , h, color on, color off, image, outline style) .
  button[0].initButtonG(&tft, tft.width() / 2, 100, onButtonWidth, onButtonHeight, TFT_RED, TFT_GREEN, onButton, 0);
  button[1].initButtonG(&tft, tft.width() / 2 - 110, 250, volWidth, volHeight, TFT_BLUE, TFT_WHITE, vol_dw, 2);
  button[2].initButtonG(&tft, tft.width() / 2 + 110, 250, volWidth, volHeight, TFT_BLUE, TFT_WHITE, vol_up, 2);
  button[3].initButtonG(&tft, tft.width() / 2, 400, volWidth, volHeight, TFT_WHITE, TFT_GREY, vol_mute, 2);
  button[0].drawButtonG(0);
  button[1].drawButtonG(0);
  button[2].drawButtonG(0);
  button[3].drawButtonG(0);

  bargraph(level, TFT_BLUE);
}

//------------------------------------------------------------------------------------------
void loop() {
  uint16_t t_x, t_y; // To store the touch coordinates
  boolean pressed = tft.getTouch(&t_x, &t_y);

  for (uint8_t i = 0; i < 4; i++) {
    if (pressed && button[i].contains(t_x, t_y))  button[i].press(true);  // tell the button it is pressed
    else button[i].press(false);  // tell the button it is NOT pressed
  }

  // *** POWER ***
  if (button[0].justPressed())  {
    power = ! power;
    button[0].drawButtonG(power);  // draw invert
  }

  // *** VOLUME UP ***
  if (button[1].justReleased()) button[1].drawButtonG();
  if (mute && level > 0 && button[1].justPressed())  {
    level--;
    button[1].drawButtonG(true);  // draw invert
    bargraph(level, TFT_BLUE);
  }

  // *** VOLUME DOWN ***
  if (button[2].justReleased()) button[2].drawButtonG();
  if (mute && level < 10 && button[2].justPressed())  {
    level++;
    button[2].drawButtonG(true);  // draw invert
    bargraph(level, TFT_BLUE);
  }


  // *** MUTE ***
  if (button[3].justPressed())  {
    mute = ! mute;
    button[3].drawButtonG(mute);  // draw invert
    uint16_t color = (mute) ? TFT_BLUE : TFT_DARKGREY;
    bargraph(level, color);
  }

}


// -----------------------------------------
void bargraph(uint8_t l, uint16_t col) {
  uint16_t xpos = 110;
  uint16_t ypos = 270;
  uint16_t xinc = 10;
  uint16_t yinc = 5;

  for (uint8_t i = 0; i < 10; i++) {
    uint16_t color = (level > i) ? col : TFT_BLACK;
    tft.fillRoundRect(xpos + xinc * i, ypos - yinc * i, 8, 10 + yinc * i, 2, color);
    tft.drawRoundRect(xpos + xinc * i, ypos - yinc * i, 8, 10 + yinc * i, 2, TFT_WHITE);
  }
}


// -----------------------------------------
void touch_calibrate() {
  uint16_t calData[5];
  uint8_t calDataOK = 0;

  // check file system exists
  if (!SPIFFS.begin()) {
    Serial.println("Formating file system");
    SPIFFS.format();
    SPIFFS.begin();
  }

  // check if calibration file exists and size is correct
  if (SPIFFS.exists(CALIBRATION_FILE)) {
    if (REPEAT_CAL) {
      // Delete if we want to re-calibrate
      SPIFFS.remove(CALIBRATION_FILE);
    }
    else
    {
      File f = SPIFFS.open(CALIBRATION_FILE, "r");
      if (f) {
        if (f.readBytes((char *)calData, 14) == 14)
          calDataOK = 1;
        f.close();
      }
    }
  }

  if (calDataOK && !REPEAT_CAL) {
    // calibration data valid
    tft.setTouch(calData);
  } else {
    // data not valid so recalibrate
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 0);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.println("Touch corners as indicated");

    tft.setTextFont(1);
    tft.println();

    if (REPEAT_CAL) {
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.println("Set REPEAT_CAL to false to stop this running again!");
    }

    tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.println("Calibration complete!");

    // store data
    File f = SPIFFS.open(CALIBRATION_FILE, "w");
    if (f) {
      f.write((const unsigned char *)calData, 14);
      f.close();
    }
  }
}
