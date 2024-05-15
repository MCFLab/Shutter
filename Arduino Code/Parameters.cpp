#include <Arduino.h>
#include <EEPROM.h>

#include "Common.h"
#include "Parameters.h"


////////////////////////////
// Constructor
Parameters::Parameters() : numShuttersDefined(0) {}

////////////////////////////
// Set the shutter parameters
// if shutter=-1, add a new shutter
int8_t Parameters::set(int8_t shutter, uint8_t shieldChannel, int8_t digInput, uint16_t posOpen, uint16_t posClosed,
                                 uint16_t transitDelay_ms, const char* label)
{
  int8_t selectedShutter;
  char* endOfStringPos;

  if (shutter==-1) { // add a new shutter
    if (numShuttersDefined>=MAXSHUTTERS) {
      return -1; // already at limit
    } else {
      selectedShutter=numShuttersDefined;
      numShuttersDefined++;
    }
  } else if (shutter>=numShuttersDefined) {
    return -1; // shutter not defined
  } else {
    selectedShutter=shutter;
  }

  params[selectedShutter].shieldChannel      = shieldChannel;
  params[selectedShutter].digInput    = digInput;
  params[selectedShutter].posOpen         = posOpen;
  params[selectedShutter].posClosed       = posClosed;
  params[selectedShutter].transitDelay_ms = transitDelay_ms;
  sprintf(params[selectedShutter].label, "%."MAXLABELCHARS_STR"s", label);
  return 0;
}

////////////////////////////
// Clear the shutter info
void Parameters::clear(void)
{
  numShuttersDefined=0;
}

////////////////////////////
// Save the info to EEPROM
int8_t Parameters::saveToEEPROM(void)
{
  int eeAddress = 0;

  if (numShuttersDefined==0) return -1; // no shutters defined

  EEPROM.put(eeAddress, numShuttersDefined);
  eeAddress += sizeof(int8_t);
  for (int8_t idx=0; idx<numShuttersDefined; idx++){
    EEPROM.put(eeAddress, params[idx]);
    eeAddress += sizeof(ShutterStruct);
  }
  return 0;
}

////////////////////////////
// Read the info from EEPROM
int8_t Parameters::readFromEEPROM(void)
{
  int eeAddress = 0;

  EEPROM.get(eeAddress, numShuttersDefined);
  if (numShuttersDefined==0 || numShuttersDefined>MAXSHUTTERS) { // no shutters defined
   numShuttersDefined=0;
   return -1;
  } 
  eeAddress += sizeof(int8_t);
  for (int8_t idx=0; idx<numShuttersDefined; idx++){
    EEPROM.get(eeAddress, params[idx]);
    params[idx].label[MAXLABELCHARS-1]='\0'; // just in case there was garbage in the EEPROM
    eeAddress += sizeof(ShutterStruct);
  }
  return 0;
}

////////////////////////////
// Get the values for the members
int8_t Parameters::numShutters(void)
{
  return numShuttersDefined;
}
uint8_t Parameters::shieldChannel(int8_t shutter)
{
  return params[shutter].shieldChannel;
}
int8_t Parameters::digInput(int8_t shutter)
{
  return params[shutter].digInput;
}
uint16_t Parameters::posOpen(int8_t shutter)
{
  return params[shutter].posOpen;
}
uint16_t Parameters::posClosed(int8_t shutter)
{
  return params[shutter].posClosed;
}
uint16_t Parameters::transitDelay(int8_t shutter)
{
  return params[shutter].transitDelay_ms;
}

////////////////////////////
// Return the label (no special formatting)
int8_t Parameters::getLabel(int8_t shutter, char* label)
{
  if (shutter<numShuttersDefined) {
    sprintf(label, "%."MAXLABELCHARS_STR"s", params[shutter].label);
    return 0;
  } else {
    return -1;
  }
}

////////////////////////////
// Return the label with a minimum width, left-aligned, right-filled with spaces
int8_t Parameters::getPrintLabel(int8_t shutter, char* label)
{
  if (shutter<numShuttersDefined) {
    sprintf(label, "%-"MAXLABELCHARS_STR"."MAXLABELCHARS_STR"s", params[shutter].label);
    return 0;
  } else {
    return -1;
  }
}
