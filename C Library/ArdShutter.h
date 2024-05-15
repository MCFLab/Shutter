// *****************************************************************************************
//
// Header file for the utility functions for the Arduino
//
// *****************************************************************************************


// *****************************************************************************************
// Exported function prototypes
// *****************************************************************************************
// Initialization
int ARD_ShutterInit(const char *address);

// Close function
int ARD_Close(void);

// Get number of attached device
int ARD_ShutterGetNumDevices(int *numDevices);

// Clear device parameters
int ARD_ShutterClearDev(void);

// Get shutter state
//   device: the shutter attached to the Arduino
//   state: 0->Closed, 1->Open
int ARD_ShutterGetState(int device, int *state);

// Set shutter state
//   device: the shutter attached to the Arduino
//   state: 0->Closed, 1->Open
int ARD_ShutterSetState(int device, int state);

// Get shutter device label
//   device: the shutter attached to the Arduino
int ARD_ShutterGetDeviceLabel(int device, char *label);

// Get shutter transit delay
//   device: the shutter attached to the Arduino
//   transit time in ms
int ARD_ShutterGetTransitDelay(int device, int *transDelay_ms);

// Set shutter position
//   device: the shutter attached to the Arduino
//   position: PWM value
int ARD_ShutterSetPosition(int device, int PWMVal);

// Get shutter parameters
//   device: the shutter attached to the Arduino
//   shieldChannel: actuator chanel on the shield
//   digIn: the number of the digital input (-1 for none)
//   open/closedPos: actuator value for the respective positions
//   transitDelay_ms: time for the shutter to open/close in ms
//   label: label to display
int ARD_ShutterGetParameters(int device, int *PWMChannel, int *digInput, int *openPos,
														 int *closedPos, int *transitDelay_ms, char* label);

// Set shutter parameters
//   device: the shutter attached to the Arduino
//   shieldChannel: chanel on the shield
//   digIn: the number of the digital input (-1 for none)
//   open/closedPos: actuator value for the respective positions
//   transitDelay_ms: time for the shutter to open/close in ms
//   label: label to display
int ARD_ShutterSetParameters(int device, int PWMChannel, int digInput, int openPos,
														 int closedPos, int transitDelay_ms, const char* label);

// Save parameters to EEPROM
int ARD_ShutterSaveToEEPROM(void);
