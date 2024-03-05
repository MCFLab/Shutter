#include "Common.h"
// only include if DIGINPUT is defined in "Common.h"
#ifdef DIGINPUT


#ifndef DIGINPUT_H
#define DIGINPUT_H

#define CHECK_INTERVAL_MS 5   // interval in ms for the bounce check
#define MAX_CHECKS        10  // number of intervals in a bounce check cycle

class DigInput
{
//private:
public:
  static inline uint8_t status; // status flag (here only bounce finished bit) 
  static inline uint8_t lowState, highState; // debounced state
  static inline uint8_t state[MAX_CHECKS];
  static inline uint8_t index;

  DigInput();
  Begin();
  uint8_t CheckState(uint8_t *pinState);
};

#endif // DIGINPUT_H

#endif // DIGINPUT