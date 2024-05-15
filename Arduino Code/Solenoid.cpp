#include "Common.h"
// only include if SHUTTER_SOLENOID is chosen in "Common.h"
#ifdef SHUTTER_SOLENOID


#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "Solenoid.h"

#define SERIAL_DEBUG  0


// *************************************************************************************
// defines
// *************************************************************************************

// *************************************************************************************
// global variables
// *************************************************************************************
Adafruit_MotorShield _AFMS = Adafruit_MotorShield(SOLENOID_BOARDID);
Adafruit_DCMotor* _motorPtr[4]; // four motors per board

// *************************************************************************************
// RCServo class
// *************************************************************************************
////////////////////////////
// Constructor
Solenoid::Solenoid()
{
}

////////////////////////////
// Initialize stuff
Solenoid::Begin()
{
#if SERIAL_DEBUG>0
  Serial.println(F("Solenoid Begin."));
#endif      

  // set up motor board
  for (uint8_t ind=0; ind<4; ind++) {
    _motorPtr[ind] = _AFMS.getMotor(ind+1); // motor index is 1-based
  }

  if (!_AFMS.begin()) {         // create with the default frequency 1.6KHz
#if SERIAL_DEBUG>0
  Serial.println(F("Could not find Motor Shield. Check wiring."));
#endif      
    while (1);
  }

  // Initially, turn all motors off
  for (uint8_t ind=0; ind<4; ind++) {
    _motorPtr[ind]->run(RELEASE);
  }

}

////////////////////////////
// Action functions
Solenoid::SetShutterValue(uint8_t dev, uint16_t value)
{
  // Make sure the values are in range
  if (value>255) value = 255;
  if (dev>3) {
#if SERIAL_DEBUG>0
    Serial.println(F("Illegal motor device number (0-3)."));
#endif      
    return;
  }

  // Set the force of the solenoid, from 0 (off) to 255 (max)
  if (value>0) { 
    _motorPtr[dev]->setSpeed(value);
    _motorPtr[dev]->run(FORWARD);
  } else {
    _motorPtr[dev]->run(RELEASE);
  }
#if SERIAL_DEBUG>0
  Serial.print(F("Setting motor ")); Serial.print(dev); Serial.print(" to ");
  Serial.print(value); Serial.println(".");
#endif      
}

#endif // SHUTTER_SOLENOID