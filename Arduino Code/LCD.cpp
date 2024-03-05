#include "Common.h"
// only include if LCD is chosen in "Common.h"
#ifdef DISPLAY_LCD


#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>
#include "LCD.h"

#define SERIAL_DEBUG  0


// *************************************************************************************
// defines
// *************************************************************************************
// defines for the LCD shield
#define LCD_OFF 0x0
#define LCD_ON  0x1
#define SPACE_CHAR  32
#define ARROW_CHAR  126


// *************************************************************************************
// global variables
// *************************************************************************************
Adafruit_RGBLCDShield _lcdDev = Adafruit_RGBLCDShield();


// *************************************************************************************
// LCD class
// *************************************************************************************
////////////////////////////
// Constructor
LCD::LCD(unsigned int dispTurnoffInterval_s) : dispTurnoffInterval_s{dispTurnoffInterval_s}
{
}

////////////////////////////
// Initialize stuff
LCD::Begin()
{
#if SERIAL_DEBUG>0
  Serial.println(F("LCD Begin."));
#endif      

  // set up the LCD's number of columns and rows: 16 columns, 2 rows
  _lcdDev.begin(16, 2);
  _lcdDev.setBacklight(LCD_ON);

  // print 'LCD mask' 
  _lcdDev.clear();
  _lcdDev.setCursor(0,1);
  _lcdDev.print(" Closed   Open");
}

////////////////////////////
// Set the members
LCD::SetNumDevs(int8_t numShutters)
{
  numDevs = numShutters;
  if (numDevs>0) {
    currDevice = 0;
    RefreshDisplay();
  } else {
    currDevice = -1;
    char emptyLabel[MAXLABELCHARS+1];
    sprintf(emptyLabel, "%-"MAXLABELCHARS_STR"s", "---");
    _lcdDev.setCursor(0,0); _lcdDev.print(emptyLabel);
  }
}
LCD:: SetDevText(int8_t device, char *label)
{
  sprintf(devLabels[device], "%-"MAXLABELCHARS_STR"s", label);
}

////////////////////////////
// Refresh the entire display
LCD::RefreshDisplay()
{
  if (currDevice>=0) {
    _lcdDev.setCursor(0,0); _lcdDev.print(devLabels[currDevice]);
    RefreshDev(currDevice);
  }
}

////////////////////////////
// Refresh state display of device
LCD::RefreshDev(int8_t device)
{
  if (device==currDevice) { // only update if actually displayed
    if (devState[device]==0) {
      _lcdDev.setCursor(0,1); _lcdDev.write(ARROW_CHAR);
      _lcdDev.setCursor(9,1); _lcdDev.write(SPACE_CHAR);
    } else if (devState[device]==1) {
      _lcdDev.setCursor(0,1); _lcdDev.write(SPACE_CHAR);
      _lcdDev.setCursor(9,1); _lcdDev.write(ARROW_CHAR);
    } else if (devState[device]==-1) {
      _lcdDev.setCursor(0,1); _lcdDev.write(SPACE_CHAR);
      _lcdDev.setCursor(9,1); _lcdDev.write(SPACE_CHAR);
    }
  }
}

////////////////////////////
// Change the stat of the device
// state is on (1), off (0), or undefined (-1)
LCD::ChangeDevState(int8_t device, int8_t state)
{
  devState[device] = state;
  RefreshDev(device);
}

////////////////////////////
// Check for buttons
// return 1 if it was a valid request
// returns the device that requests the changed state by reference
int8_t LCD::CheckInput(int8_t *device, int8_t *state)
{
  unsigned long currentTime = millis();
  uint8_t buttons;

  // do nothing if no device is defined
  if (numDevs<=0) return 0;

  // set invalid defaults
  *device = -1;
  *state = -1;

  if (currentTime - lastButtonTime > LCD_BLOCKING_TIME_MS) {
    if (isDisplayOff) {
      if(_lcdDev.readButtons()) {
        Wakeup();
        lastButtonTime = currentTime;
        isDisplayOff = 0;
      }
    } else { // display is still on
      if ( dispTurnoffInterval_s > 0
            && currentTime - lastButtonTime > 1000*(unsigned long) dispTurnoffInterval_s ) {
        Sleep();
        isDisplayOff = 1;
      } else { // check for button input
        if (buttons = _lcdDev.readButtons()) {
          lastButtonTime = currentTime;

          if (buttons & BUTTON_UP) {
            currDevice++;
            if (currDevice==numDevs) currDevice=0;
            RefreshDisplay();
          }
          if (buttons & BUTTON_DOWN) {
            if (currDevice==0) currDevice=numDevs;
            currDevice--;
            RefreshDisplay();
          }
          if (buttons & BUTTON_LEFT) {
            *device = currDevice;
            *state = 0;
#if SERIAL_DEBUG>0
            Serial.print("Device "); Serial.print(*device); Serial.print(" requested state "); Serial.println(*state);
#endif
            return 1;
          }
          if (buttons & BUTTON_RIGHT) {
            *device = currDevice;
            *state = 1;
#if SERIAL_DEBUG>0
            Serial.print("Device "); Serial.print(*device); Serial.print(" requested state "); Serial.println(*state);
#endif
            return 1;
          }
          if (buttons & BUTTON_SELECT) {
            // no action at the moment
          }
        }
      }
    }
  }
  return 0;
}

////////////////////////////
// Put the display to sleep
LCD::Sleep()
{
  _lcdDev.setBacklight(LCD_OFF);
}

////////////////////////////
// Wake up the display
LCD::Wakeup()
{
  _lcdDev.setBacklight(LCD_ON);
}

#endif // DISPLAY_LCD