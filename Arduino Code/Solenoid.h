#include "Common.h"
// only include if SHUTTER_SOLENOID is chosen in "Common.h"
#ifdef SHUTTER_SOLENOID


#ifndef SOLENOID_H
#define SOLENOID_H

class Solenoid
{
public:
  Solenoid();
  Begin();
  SetShutterValue(uint8_t dev, uint16_t value);
};

#endif // SOLENOID_H

#endif // SHUTTER_SOLENOID