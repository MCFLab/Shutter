#include "Common.h"
// only include if SHUTTER_RCSERVO is chosen in "Common.h"
#ifdef SHUTTER_RCSERVO


#ifndef RCSERVO_H
#define RCSERVO_H

class RCServo
{
public:
  RCServo();
  Begin();
  SetShutterValue(uint8_t dev, uint16_t value);
};

#endif // RCSERVO_H

#endif // SHUTTER_RCSERVO