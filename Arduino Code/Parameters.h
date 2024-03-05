#ifndef PARAMETERS_H
#define PARAMETERS_H

struct ShutterStruct {
  uint8_t PWMChannel;
  int8_t digInput;
  uint16_t posClosed;
  uint16_t posOpen;
  uint16_t transitDelay_ms;
  char label[MAXLABELCHARS+1];
};

class Parameters
{
	int8_t numShuttersDefined = 0;      // shutters in use
  ShutterStruct params[MAXSHUTTERS];

  public:
  Parameters();

  // set the shutter parameters for the indicated shutter
  //    shutter: pass -1 to create new shutter
  //    label: anything longer than MAXLABELCHARS will be cut off
  int8_t set(int8_t shutter, uint8_t PWMChannel, int8_t digInput, uint16_t openPos, uint16_t closePos,
               uint16_t transitDelay_ms, const char* label);

  // clear the shutter info
  void clear(void);

  // Read/Write the info from/to the EEPROM
  int8_t saveToEEPROM(void);
  int8_t readFromEEPROM(void);

  // Return the number of shutters
  int8_t numShutters(void);

  // Return various parameters for the indicated shutter
  uint8_t PWMChannel(int8_t shutter);
  int8_t digInput(int8_t shutter);
  uint16_t posOpen(int8_t shutter);
  uint16_t posClosed(int8_t shutter);
  uint16_t transitDelay(int8_t shutter);

  // Return the label (no special formatting)
  int8_t getLabel(int8_t shutter, char* label);

  // Return the label with a minimum width, left-aligned, right-filled with spaces
  int8_t getPrintLabel(int8_t shutter, char* label);

};

#endif