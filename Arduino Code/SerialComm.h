#include "Common.h"
// only include if SERIALCOMM is chosen in "Common.h"
#ifdef SERIALCOMM

#ifndef SERIALCOMM_H
#define SERIALCOMM_H

#include "Parameters.h"

typedef enum {
  None = 0,
  StateChange,
  ManualPos,
  ParamChange
} SerialActionType;


// *************************************************************************************
// SerialComm class
// *************************************************************************************
class SerialComm
{
private:
  Parameters *params;
  int8_t *devState;
public:
  SerialComm();
  Begin(Parameters *paramPtr, int8_t *devStatePtr, long timeout_ms = 1000);
  CheckAction(SerialActionType *action, int8_t *device, int8_t *state, uint16_t *manPos);
};

#endif // SERIALCOMM_H

#endif // SERIALCOMM
