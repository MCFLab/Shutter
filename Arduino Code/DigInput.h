#include "Common.h"
// only include if DIGINPUT is defined in "Common.h"
#ifdef DIGINPUT


#ifndef DIGINPUT_H
#define DIGINPUT_H

class DigInput
{
//private:
public:
  static inline uint8_t status; // status flag (here only bounce finished bit) 
  static inline uint8_t lowState, highState; // debounced state
  static inline uint8_t state[DIGINPUT_MAX_CHECKS];
  static inline uint8_t index;

  DigInput();
  Begin();
  uint8_t CheckState(uint8_t *pinState);
};

#endif // DIGINPUT_H

#endif // DIGINPUT