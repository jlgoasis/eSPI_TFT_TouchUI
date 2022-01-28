/*
   Example of drawing a "switch" and using
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

#include "TFT_eSPI_TouchUI.h"

// Invoke custom library
TFT_eSPI tft = TFT_eSPI();

// This is the file name used to store the touch coordinate
// calibration data. Cahnge the name to start a new calibration.
#define CALIBRATION_FILE "/TouchCalData0"

// Set REPEAT_CAL to true instead of false to run calibration
// again, otherwise it will only be done once.
// Repeat calibration if you change the screen rotation.
#define REPEAT_CAL false

// Invoke the TFT_eSPI Slider class and create all the slider objects
TFT_eSPI_TouchUI button[10];

char* keyLabel[10] = {"1", "2", "3", "4", "5", "6", "T", "R", "B", "L"};

uint16_t outlineColor[6] = {TFT_YELLOW, TFT_GREEN, TFT_WHITE, TFT_WHITE, TFT_GREEN, TFT_WHITE};
uint16_t onColor[6] =    {TFT_GREEN, TFT_GREEN, TFT_BLUE, TFT_BLUE, TFT_RED, TFT_DARKGREY};
uint16_t offColor[6] =    {TFT_DARKGREY, TFT_BLUE, TFT_CYAN, TFT_DARKCYAN, TFT_BLUE, TFT_YELLOW};

uint8_t w[6] = {70, 80, 90, 80, 80, 90};
uint8_t h[6] = {70, 60, 60, 80, 80, 70};

uint16_t xpos[10] = {10, 120, 220, 10, 120, 220, 160, 160 + 70, 160, 160 - 70};
uint16_t ypos[10] = {40, 40, 40, 150, 150, 150, 360 - 70, 360, 360 + 70, 360};

uint16_t b = 5;

//------------------------------------------------------------------------------------------
void setup(void) {
  Serial.begin(115200);
  Serial.println();

  tft.init();

  // Set the rotation before we calibrate
  tft.setRotation(0);

  // call screen calibration
  touch_calibrate();

  // clear screen
  tft.fillScreen(TFT_BLACK);

  tft.fillRect(0, 0, tft.width(), 20, TFT_BLUE);
  tft.setTextColor(TFT_WHITE, TFT_BLUE);
  tft.drawCentreString("BUTTON TEST", tft.width() / 2, 2, 2);

  tft.setFreeFont(&FreeSans12pt7b);

  // Buttons 0 -3 (Left/Top x/y)
  for (uint8_t i = 0; i < 6; i++) {
    /* Init parameters:
    (&tft, uint16_t x, uint16_t y , uint16_t w, uint16_t h, uint16_t outline color, uint16_t on color, uint16_t off color, char * label, uint8_t knob type)
    knob type:  0: No draw, only label
                1: Rectangle
                2: Rentangle with border
                3: Rounded rectangle
                4: Circle
                5: Circle with border
                6: Rectangle led
                7: Triangle Top
                8: Triangle Right
                9: Triangle Button
                10: Triangle Left   
    */            
    button[i].initButtonUL(&tft, xpos[i], ypos[i], w[i], h[i], outlineColor[i], onColor[i], offColor[i], keyLabel[i], i + 1);
    button[i].drawButton();  // false
  }

  // Triangle buttons (centre x/y)
  for (uint8_t i = 6; i < 10; i++) {
    // &tft, min value, max value, x, y , w, l,outline color, fill color, text color, label, knob type.
    button[i].initButton(&tft, xpos[i], ypos[i], 64, 64, TFT_WHITE, TFT_DARKGREY, TFT_GREEN, keyLabel[i], i + 1);
    button[i].drawButton();
  }
}


//------------------------------------------------------------------------------------------
void loop() {
  uint16_t t_x, t_y; // To store the touch coordinates

  // Pressed will be set true is there is a valid touch on the screen
  boolean pressed = tft.getTouch(&t_x, &t_y);
  tft.setFreeFont(&FreeSans12pt7b);
  // / Check if any key coordinate boxes contain the touch coordinates
  for (uint8_t i = 0; i < 10; i++) {
    if (pressed && button[i].contains(t_x, t_y)) {
      button[i].press(true);  // tell the button it is pressed
    } else {
      button[i].press(false);  // tell the button it is NOT pressed
    }

    if (button[i].justReleased()) button[i].drawButton();     // draw off (false)

    if (button[i].justPressed())  {

      button[i].drawButton(true);  // draw invert

      // Draw value triangle buttons
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.setFreeFont(&FreeSans24pt7b);
      tft.setTextDatum(MC_DATUM);
      tft.setTextPadding(tft.textWidth("8"));
      if (i < 6) b = i + 1;
      if ((i == 6 || i == 7) && b < 9) b++;
      if ((i == 8 || i == 9) && b > 0) b--;
      tft.drawNumber(b, 160, 360);
    }
  }
}


//-----------------------------------------
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
