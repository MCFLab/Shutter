///////////////////////////////////////////////////////////////////////
// Shutter driver sketch
// Martin Fischer, Duke University
// Written for the Arduino platform; controls actuators that serve as
//   shutters for some laser beams.
// Uses the Adafruit TFT Touch Shield or the Adafruit LCD dislay
// Also uses the Adafruit PWM shield or motor shield
///////////////////////////////////////////////////////////////////////

////////////////////////////
// for memory diagnostics
//extern char *__brkval;
////////////////////////////

#include "Common.h"
#include "Parameters.h"
#include "RCServo.h"
#include "Solenoid.h"
#include "TFT.h"
#include "LCD.h"
#include "SerialComm.h"
#include "DigInput.h"

#define SERIAL_DEBUG  0

//************************************************
// global variables
//************************************************
Parameters _params = Parameters();
#ifdef SHUTTER_RCSERVO
RCServo _shutter = RCServo();
#endif
#ifdef SHUTTER_SOLENOID
Solenoid _shutter = Solenoid();
#endif
#ifdef DISPLAY_TFT
TFT _display = TFT();
#endif
#ifdef DISPLAY_LCD
LCD _display = LCD();
#endif
#ifdef SERIALCOMM
SerialComm _serComm = SerialComm();
#endif
#ifdef DIGINPUT
DigInput _digInput = DigInput();
#endif

static unsigned long _lastStateChangeTime_ms = 0;
static int8_t _devState[MAXSHUTTERS];


//************************************************
// functions for debugging
//************************************************
////////////////////////////
// create dummy values for the shutter parameters
////////////////////////////
void createDummyParameters(void)
{
  // default solenoid parameters
  // _params.set(-1, 0, -1, 255, 0, 500, "Label0");
  // _params.set(-1, 1, -1, 255, 0, 500, "Label1");
  // _params.set(-1, 2, -1, 255, 0, 500, "Label2");
  // _params.set(-1, 3, -1, 255, 0, 500, "Label3");

  // default servo parameters
  // _params.set(-1, 0, -1, 200, 300, 500, "Label0");
  // _params.set(-1, 1, -1, 200, 300, 500, "Label1");
  // _params.set(-1, 2, -1, 200, 300, 500, "Label2");
  // _params.set(-1, 3, -1, 200, 300, 500, "Label3");
}


//************************************************
// setup
//************************************************
void setup() {

  // initialize the array to an unused state
  for(uint8_t ind = 0; ind < sizeof(_devState); ++ind) _devState[ind] = -2;

#ifdef SERIALCOMM
  _serComm.Begin(&_params, _devState); // default timeout
#endif

  _shutter.Begin();

  // read parameter info from EEPROM
  _params.readFromEEPROM();

  // in case of erroneous behavior when first using the Arduino, it could be becasue of some strange
  // values in the EEPROM. To rule that out, comment out the above line and instead use the two lines
  // below. These save some pre-defined values into the EEPROM. Then revert back to the original
  //  config and compile/download the code again.
//  createDummyParameters();
//  _params.saveToEEPROM();

  // set up the display
#if defined DISPLAY_TFT || defined DISPLAY_LCD
  _display.Begin();
  updateDisplayInfo();
#endif

// set up the digital inputs
#ifdef DIGINPUT
  _digInput.Begin();
#endif

}


//************************************************
// loop
//************************************************
void loop() {

#if defined DISPLAY_TFT || defined DISPLAY_LCD
  checkDisplayInput();
#endif
#ifdef SERIALCOMM
  checkSerialInput();
#endif
#ifdef DIGINPUT
  checkDigitalInput();
#endif
  checkForIdle(); // disconnects the servo after a while of inactivity

}


//************************************************
// action functions
//************************************************
////////////////////////////
// check for display input
////////////////////////////
#if defined DISPLAY_TFT || defined DISPLAY_LCD
void checkDisplayInput(void)
{
   int8_t device;
   int8_t desiredState;

  if (_display.CheckInput(&device, &desiredState)) {
    updateState(device, desiredState);
  }
}
#endif

////////////////////////////
// check for serial input
////////////////////////////
#ifdef SERIALCOMM
void checkSerialInput(void)
{
  SerialActionType action;
  int8_t device;
  int8_t desiredState;
  uint16_t manualPos;

  _serComm.CheckAction(&action, &device, &desiredState, &manualPos);
  if (action == None)
    return;
  else if (action == ParamChange){
#if defined DISPLAY_TFT || defined DISPLAY_LCD
    updateDisplayInfo();
#endif
  } else if (action == StateChange)
    updateState(device, desiredState);
  else if (action == ManualPos) {
    _shutter.SetShutterValue(_params.shieldChannel(device), manualPos);
    _lastStateChangeTime_ms = millis();
    _devState[device]=2; // flag for manual set
  }

}
#endif

////////////////////////////
// check for digital input
////////////////////////////
#ifdef DIGINPUT
void checkDigitalInput(void)
{

  uint8_t portState; // byte that defines the bits in the input port (only 0-3 are used)
  int8_t digInput; // input number for the device (0-3)
  int8_t newState; // desired state of the device

  if (_digInput.CheckState(&portState))
  {
    // loop through devices
    for (int8_t dev=0; dev<_params.numShutters(); dev++) {
      digInput = _params.digInput(dev); 
      if (digInput==-1) continue; // no digital input defined for this device
      newState = ( (portState & bit(digInput)) > 0 ? 1 : 0);
      if (_devState[dev] != newState ) {
        updateState(dev, newState);
#if SERIAL_DEBUG>0
        Serial.print(F("digInput = ")); Serial.println(digInput);
        Serial.print(F("portstate = ")); Serial.println(portState);
        Serial.print(F("Dig update device ")); Serial.print(dev); Serial.print(F(" to ")); Serial.print(newState); Serial.println("");
#endif      
      }
    }
  }

}
#endif

////////////////////////////
// check for elapsed time to disable the servos
////////////////////////////
void checkForIdle(void)
{
  unsigned long currentTime;
  currentTime = millis();

  if ( IDLEINTERVAL_S > 0 
          && currentTime - _lastStateChangeTime_ms > 1000*(unsigned long)IDLEINTERVAL_S) {
    for (int8_t dev=0; dev<_params.numShutters(); dev++) {
      if (_devState[dev]==-1) continue; // already disabled
      updateState(dev, -1);
#if SERIAL_DEBUG>0
      Serial.print(F("Setting device ")); Serial.print(dev); Serial.println(" to idle.");
#endif      
      _shutter.SetShutterValue(_params.shieldChannel(dev), 0);
      _devState[dev]=-1;
    }
  }
}


//************************************************
// Utility functions
//************************************************
/////////////////////////////
// memory diagnostics
//Serial.print(F("Free memory: ")); Serial.println(freeMemory());
////////////////////////////
// set the shutters
////////////////////////////
void updateState(int8_t device, int8_t state)
{
  // only update shutter state if needed
  if (state==_devState[device]) return;

  if (state==0) { // close
    _shutter.SetShutterValue(_params.shieldChannel(device), _params.posClosed(device));
  } else if (state==1) { // open
    _shutter.SetShutterValue(_params.shieldChannel(device), _params.posOpen(device));
  } else if (state==-1) { // idle
    _shutter.SetShutterValue(_params.shieldChannel(device), 0);
  }
  _devState[device]=state;
#if defined DISPLAY_TFT || defined DISPLAY_LCD
  _display.ChangeDevState(device, state);
#endif
  _lastStateChangeTime_ms = millis();
}

////////////////////////////
// update the display
////////////////////////////
#if defined DISPLAY_TFT || defined DISPLAY_LCD
void updateDisplayInfo(void)
{
  char label[MAXLABELCHARS+1];
  int8_t numShutters = _params.numShutters();
  _display.SetNumDevs(numShutters);

  for (int8_t z = 0; z<numShutters; z++) {
    _params.getPrintLabel(z, label);
    _display.SetDevText(z, label);
  }
  _display.RefreshDisplay();
}
#endif

//////////////////////////
// memory diagnostics
// int freeMemory(void)
// {
//   char top;
//   return &top - __brkval;
// }
