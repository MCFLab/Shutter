#include "Common.h"
// only include if SERIALCOMM is chosen in "Common.h"
#ifdef SERIALCOMM


#include <Arduino.h>
#include "SerialComm.h"

#define SERIAL_DEBUG  0


// *************************************************************************************
// defines
// *************************************************************************************
#define MSG_MAXLENGTH  50

// *************************************************************************************
// global variables
// *************************************************************************************

// *************************************************************************************
// LCD class
// *************************************************************************************
////////////////////////////
// Constructor
SerialComm::SerialComm(){}

////////////////////////////
// Initialize stuff
SerialComm::Begin(Parameters *paramPtr, int8_t *devStatePtr, long timeout_ms = 1000)
{
  Serial.begin(SERIAL_BAUDRATE);
#if SERIAL_DEBUG>0
  Serial.println(F("\nSerialComm started."));
#endif      
  // note: default time out for readBytesUntil is 1000 ms, set to 100ms
  Serial.setTimeout(timeout_ms);
  params = paramPtr;
  devState = devStatePtr;
}

////////////////////////////
// Check for serial requests
SerialComm::CheckAction(SerialActionType *action, int8_t *device, int8_t *state, uint16_t *manPos)
{
  static char serialData[MSG_MAXLENGTH+1]; // one extra for the null char
  int8_t tempDev;
  int8_t tempState;
  int8_t status;
  char label[MAXLABELCHARS+1];
  int8_t shutter;
  uint8_t PWMChannel;
  int8_t digInput;
  uint16_t openPos, closePos, tempPos, transitDelay;

  *action = None;

  if (Serial.available() > 0){
    // max allowed command size is MSG_MAXLENGTH char, ends with a term char
    int bytesRead = Serial.readBytesUntil(SERIAL_TERMCHAR, serialData, MSG_MAXLENGTH); // LF or CR
    // terminate with an end character
    serialData[bytesRead]='\0';
    // check for at least some bytes
    if (bytesRead<3) {    
      Serial.println(F("Error: Commands needs to be at least 3 characters."));
      return;
    }
#if SERIAL_DEBUG>0
    Serial.println("---");
    Serial.println(serialData);
    Serial.print(F("Bytes received (")); Serial.print(bytesRead); Serial.print(")");
    for (int q=0; q<bytesRead; q++) {Serial.print(" "); Serial.print(byte(serialData[q])); }
    Serial.println();
#endif      

    /////////////////////
    // check for ID query
    if (strncmp(serialData, "*IDN?", 5)  == 0){
      Serial.println(ID_STRING);
      return;
    }

    /////////////////////
    // check for time query
    if (strncmp(serialData, "GTI", 3)  == 0){
      Serial.print("TI="); Serial.println(millis());
      return;
    }

    /////////////////////
    // check for numDev query
    if (strncmp(serialData, "GND", 3)  == 0){
      Serial.print("ND=");Serial.println(params->numShutters());
      return;
    }
    
    /////////////////////
    // check for GetShutterState command
    if (strncmp(serialData, "GST", 3)  == 0){
      if (sscanf(serialData, "GST%hhd", &tempDev)!=1) {
        Serial.println(F("Error: Invalid GST command format."));
        return;
      }
      // check the validity of the device 
      if (tempDev < 0 || tempDev>=params->numShutters()) {
        Serial.println(F("Error: Invalid device number."));
        return;
      }
      // return the state
      Serial.print("ST");Serial.print(tempDev);Serial.print("=");Serial.println(devState[tempDev]);
      return;
    }  

    /////////////////////
    // check for GetDeviceLabel command
    if (strncmp(serialData, "GDL", 3)  == 0){
      if (sscanf(serialData, "GDL%hhd", &tempDev)!=1) {
        Serial.println(F("Error: Invalid GDL command format."));
        return;
      }
      // check the validity of the device 
      if (tempDev < 0 || tempDev>=params->numShutters()) {
        Serial.println(F("Error: Invalid device number."));
        return;
      }
      params->getLabel(tempDev, label);
      Serial.print("DL");Serial.print(tempDev);Serial.print("=");Serial.println(label);
      return;
    }  

    /////////////////////
    // check for GetTransitDelay command
    if (strncmp(serialData, "GTD", 3)  == 0){
      if (sscanf(serialData, "GTD%hhd", &tempDev)!=1) {
        Serial.println(F("Error: Invalid GTD command format."));
        return;
      }
      // check the validity of the device 
      if (tempDev < 0 || tempDev>=params->numShutters()) {
        Serial.println(F("Error: Invalid device number."));
        return;
      }
      Serial.print("TD");Serial.print(tempDev);Serial.print("=");Serial.println(params->transitDelay(tempDev));
      return;
    }  

    /////////////////////
    // check for parameter clear
    if (strncmp(serialData, "CLR", 3)  == 0){
      params->clear();
      Serial.println("OK");
      *action = ParamChange;
      return;
    }

    /////////////////////
    // check for EEPROM save
    if (strncmp(serialData, "SAV", 3)  == 0){
      status = params->saveToEEPROM();
      if (status==0)
        Serial.println("OK");
      else
        Serial.println(F("Error: Save failed"));
      return;
    }

    /////////////////////
    // check for parameter get
    if (strncmp(serialData, "GPR", 3)  == 0){
      if (sscanf(serialData, "GPR%hhd", &tempDev)!=1) {
        Serial.println(F("Error: Invalid GPR command format."));
        return;
      }
      // check the validity of the device 
      if (tempDev < 0 || tempDev>=params->numShutters()) {
        Serial.println(F("Error: Invalid device number."));
        return;
      }
      Serial.print("PR");Serial.print(tempDev);Serial.print(",");
      Serial.print(params->PWMChannel(tempDev));Serial.print(",");
      Serial.print(params->digInput(tempDev));Serial.print(",");
      Serial.print(params->posOpen(tempDev));Serial.print(",");
      Serial.print(params->posClosed(tempDev));Serial.print(",");
      Serial.print(params->transitDelay(tempDev));Serial.print(",");
      params->getLabel(tempDev, label);
      Serial.println(label);
      return;
    }  

    /////////////////////
    // check for parameter set
    if (strncmp(serialData, "SPR", 3)  == 0){
      if (sscanf(serialData, "SPR%hhd,%hhu,%hhd,%u,%u,%u,%"MAXLABELCHARS_STR"s",
                   &shutter, &PWMChannel, &digInput, &openPos, &closePos, &transitDelay, label)!=7) {
        Serial.println(F("Error: Invalid SPR command format."));
        return;
      }
#if SERIAL_DEBUG>0
      Serial.print(F("shutter="));Serial.print(shutter);
      Serial.print(F(" PWMChannel="));Serial.print(PWMChannel);
      Serial.print(F(" digInput="));Serial.print(digInput);
      Serial.print(F(" openPos="));Serial.print(openPos);
      Serial.print(F(" closePos="));Serial.print(closePos);
      Serial.print(F(" transitDelay="));Serial.print(transitDelay);
      Serial.print(F(" label="));Serial.println(label);
#endif      
      status = params->set(shutter, PWMChannel, digInput, openPos, closePos, transitDelay, label);
      if (status==0) {
        Serial.println("OK");
      *action = ParamChange;
      } else {
        Serial.println(F("Error: Could not set shutter parameters."));
      }
      return;
    }

    /////////////////////
    // check for SetShutterState command
    if (strncmp(serialData, "SST", 3)  == 0){
      if (sscanf(serialData, "SST%hhd,%hhd", &tempDev, &tempState)!=2) {
        Serial.println(F("Error: Invalid SST command format."));
        return;
      }
#if SERIAL_DEBUG>0
      Serial.print(F("Requesting device "));Serial.print(tempDev);
      Serial.print(F(" set state to "));Serial.println(tempState);
#endif      
      // check the validity of the device and state 
      if (tempDev < 0 || tempDev>=params->numShutters()) {
        Serial.println(F("Error: Invalid device number."));
        return;
      }
      if (tempState<0 || tempState>1) {
        Serial.println(F("Error: Invalid state."));
        return;
      }
      // change the state
      *action = StateChange;
      *device = tempDev;
      *state = tempState;
      Serial.println("OK");
      return;
    }  

    /////////////////////
    // check for SetShutterPosition command
    if (strncmp(serialData, "SSP", 3)  == 0){
      // check for integer input
      if (sscanf(serialData, "SSP%hhd,%u", &tempDev, &tempPos)!=2) {
        Serial.println(F("Error: Invalid SSP command format."));
        return;
      }
#if SERIAL_DEBUG>0
      Serial.print(F("Requesting device "));Serial.print(tempDev);
      Serial.print(F(" set position to "));Serial.println(tempState);
#endif      
      // check the validity of the device 
      if (tempDev < 0 || tempDev>=params->numShutters()) {
        Serial.println(F("Error: Invalid device number."));
        return;
      }
      *action = ManualPos;
      *device = tempDev;
      *manPos = tempPos;
      Serial.println("OK");
      return;
    }  
    
    /////////////////////
    // if we ever get to here, it was an unrecognized command
    Serial.println(F("Error: Unrecognized command"));

  } // if (Serial.available() > 0) 
}


#endif // SERIALCOMM