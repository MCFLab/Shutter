#include "Common.h"
// only include if SHUTTER_RCSERVO is chosen in "Common.h"
#ifdef SHUTTER_RCSERVO


#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include "RCServo.h"

#define SERIAL_DEBUG  0


// *************************************************************************************
// defines
// *************************************************************************************

// *************************************************************************************
// global variables
// *************************************************************************************
Adafruit_PWMServoDriver _pwm = Adafruit_PWMServoDriver(RCSERVO_BOARDID);


// *************************************************************************************
// RCServo class
// *************************************************************************************
////////////////////////////
// Constructor
RCServo::RCServo()
{
}

////////////////////////////
// Initialize stuff
RCServo::Begin()
{
#if SERIAL_DEBUG>0
  Serial.println(F("RCServo Begin."));
#endif      

  // set up PWM chip 
  _pwm.begin();
  _pwm.setOscillatorFrequency(25000000);  // Reference frequency of the PWM chip
  _pwm.setPWMFreq(RCSERVO_FREQ);

}

////////////////////////////
// Action functions
RCServo::SetShutterValue(uint8_t dev, uint16_t value)
{
  _pwm.setPWM(dev, 0, value);
#if defined SERIALCOMM && SERIAL_DEBUG>0
  Serial.print(F("Setting PWM ")); Serial.print(dev); Serial.print(" to ");
  Serial.print(0); Serial.print("/"); Serial.print(value); Serial.println(".");
#endif      
}

#endif // SHUTTER_RCSERVO