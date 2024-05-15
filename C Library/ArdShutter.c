// *****************************************************************************************
//
// Utility functions for the Arduino Shutter
//
// *****************************************************************************************

#include <ansi_c.h>
#include <visa.h>
#include "ArdShutter.h"


// *****************************************************************************************
// Defines
// *****************************************************************************************
#define ARD_SHUTTER_RESPONSE	"Arduino Uno Shutter" // beginning of the response string
#define SERIAL_BAUDRATE	9600
#define SERIAL_TERMCHAR	0xA

// *****************************************************************************************
// Global variables
// *****************************************************************************************
static ViSession _resManager = 0;
static ViSession _io = 0;
static ViStatus _status;


// *****************************************************************************************
// Internal function prototypes
// *****************************************************************************************
static void reportError(int line, const char* function, char* description );
static void reportVisaError(int line, const char* function, ViSession instr, ViStatus errStatus );
static void reportARDError(int line, const char* function, char* description );
static int getDeviceParameterInt(ViSession io, const char *cmd, int device, int *param);
static int getDeviceParameterString(ViSession io, const char *cmd, int device, char *str);
static int setDeviceParameterInt(ViSession io, const char *cmd, int device, int param);
static int checkErrorResponse(ViSession io);


// *****************************************************************************************
// Exported functions
// *****************************************************************************************

////////////////////////////////////////////////////////
// Initialization
////////////////////////////////////////////////////////
int ARD_ShutterInit(const char *address)
{
	unsigned char instrResp[256];
	ViUInt32 charsRead;
	int isLocked=0;

	if (_io != 0) {
		reportError (__LINE__-2, __func__, "Shutter controller already open.");
		goto fail;
	}
	
	_status = viOpenDefaultRM(&_resManager);
	if(_status) {
		reportError (__LINE__-2, __func__, "Could not get access to the VISA resource manager.");
		goto fail;
	}
	
	_status = viOpen (_resManager, address, VI_NULL, 1000, &_io);
	if(_status) {
		reportVisaError (__LINE__-2, __func__, _io, _status);
		goto fail;
	}

	_status = viLock (_io, VI_EXCLUSIVE_LOCK, 5000, VI_NULL, VI_NULL);
	if(_status) {
		reportVisaError (__LINE__-2, __func__, _io, _status);
		goto fail;
	}
	isLocked=1;
	
	viSetAttribute(_io, VI_ATTR_ASRL_BAUD, SERIAL_BAUDRATE);
	viSetAttribute(_io, VI_ATTR_ASRL_DATA_BITS, 8);
	viSetAttribute(_io, VI_ATTR_ASRL_STOP_BITS, VI_ASRL_STOP_ONE);
	viSetAttribute(_io, VI_ATTR_ASRL_PARITY, VI_ASRL_PAR_NONE);
	viSetAttribute(_io, VI_ATTR_ASRL_FLOW_CNTRL, VI_ASRL_FLOW_NONE);
	viSetAttribute(_io, VI_ATTR_TERMCHAR, SERIAL_TERMCHAR);
	viSetAttribute(_io, VI_ATTR_ASRL_END_IN, VI_ASRL_END_TERMCHAR);

	// give Arduino has time to reset (not needed if reset jumper is shorted)
//	Delay(1.6);	
	
	// ask for identification; here I use printf/read because of the spaces in the return string
	_status = viPrintf(_io, "*IDN?\n");
	if(_status) {
		reportVisaError (__LINE__-2, __func__, _io, _status);
		goto fail;
	}
	_status = viRead (_io, instrResp, 256, &charsRead);
	if(_status) {
		reportVisaError (__LINE__-2, __func__, _io, _status);
		goto fail;
	}
	if (charsRead<2) {
		reportError (__LINE__-6, __func__, "No ID response received.");
		goto fail;
	}
	instrResp[charsRead-2]='\0';	
//	printf("%s\n", instrResp);
	if ( strncmp ((char *)instrResp, ARD_SHUTTER_RESPONSE, strlen(ARD_SHUTTER_RESPONSE)) != 0) {
		reportError (__LINE__-1, __func__, "Device is not a shutter driver.");
		goto fail;
	}
	
	_status = viUnlock (_io);
	if(_status) {
		reportVisaError (__LINE__-2, __func__, _io, _status);
		goto fail;
	}
	
	return 0;
	
fail:

	if (_io) {
		if (isLocked) viUnlock(_io);
		ARD_Close();
		_io=0;
	}
	return -1;
}


////////////////////////////////////////////////////////
// Close funct_ion
////////////////////////////////////////////////////////
int ARD_Close(void)
{
	if (_io) {
		_status = viClose(_io);
		if(_status) {
			reportVisaError (__LINE__-2, __func__, _io, _status);
			goto fail;
		}
		_io = 0; // set handle to zero
	}
	
	if (_resManager) {
		_status = viClose(_resManager);
		if(_status) {
			reportError (__LINE__-2, __func__, "Unable to close resource manager.");
			goto fail;
		}
		_resManager = 0; // set handle to zero
	}

	return 0;
fail:
	return -1;
}

////////////////////////////////////////////////////////
// Get number of attached device
////////////////////////////////////////////////////////
int ARD_ShutterGetNumDevices(int *numDevices)
{
	if (!_io) {
		reportError (__LINE__-2, __func__, "Device not open.");
		goto fail;
	}

	_status = viQueryf(_io, "GND\n", "ND=%d", numDevices ); // no return error possible
	if(_status) {
		reportVisaError (__LINE__-2, __func__, _io, _status);
		goto fail;
	}

	return 0;
fail:
	return -1;
}


////////////////////////////////////////////////////////
// Clear the device parameters
////////////////////////////////////////////////////////
int ARD_ShutterClearDev(void)
{
	int isLocked=0;

	if (!_io) {
		reportError (__LINE__-2, __func__, "Device not open.");
		goto fail;
	}

		_status = viLock (_io, VI_EXCLUSIVE_LOCK, 5000, VI_NULL, VI_NULL);
	if(_status) {
		reportVisaError (__LINE__-2, __func__, _io, _status);
		goto fail;
	}
	isLocked=1;

	_status = viPrintf(_io, "CLR\n");
	if(_status) {
		reportVisaError (__LINE__-2, __func__, _io, _status);
		goto fail;
	}
	if(checkErrorResponse(_io)!=0) {
		reportError (__LINE__-1, __func__, "ARD error:");
		goto fail;
	}

	_status = viUnlock (_io);
	if(_status) {
		reportVisaError (__LINE__-2, __func__, _io, _status);
		goto fail;
	}
	
	return 0;

fail:
	if (isLocked) viUnlock(_io);
	return -1;
}


////////////////////////////////////////////////////////
// Get shutter state
//   device: the shutter attached to the Arduino
//   state: 0->Closed, 1->Open
////////////////////////////////////////////////////////
int ARD_ShutterGetState(int device, int *state)
{
	int isLocked=0;

	if (!_io) {
		reportError (__LINE__-2, __func__, "Device not open.");
		goto fail;
	}

	_status = viLock (_io, VI_EXCLUSIVE_LOCK, 5000, VI_NULL, VI_NULL);
	if(_status) {
		reportVisaError (__LINE__-2, __func__, _io, _status);
		goto fail;
	}
	isLocked=1;

	if (getDeviceParameterInt(_io, "ST", device, state)) {
		reportError (__LINE__-1, __func__, "Could not get shutter state.");
		goto fail;
	}

	_status = viUnlock (_io);
	if(_status) {
		reportVisaError (__LINE__-2, __func__, _io, _status);
		goto fail;
	}
	
	return 0;

fail:
	if (isLocked) viUnlock(_io);
	return -1;
}


////////////////////////////////////////////////////////
// Get shutter device label
//   device: the shutter attached to the Arduino
////////////////////////////////////////////////////////
int ARD_ShutterGetDeviceLabel(int device, char *label)
{
	int isLocked=0;

	if (!_io) {
		reportError (__LINE__-2, __func__, "Device not open.");
		goto fail;
	}

	_status = viLock (_io, VI_EXCLUSIVE_LOCK, 5000, VI_NULL, VI_NULL);
	if(_status) {
		reportVisaError (__LINE__-2, __func__, _io, _status);
		goto fail;
	}
	isLocked=1;

	if (getDeviceParameterString(_io, "DL", device, label)) {
		reportError (__LINE__-1, __func__, "Could not get shutter label.");
		goto fail;
	}
		
	_status = viUnlock (_io);
	if(_status) {
		reportVisaError (__LINE__-2, __func__, _io, _status);
		goto fail;
	}
	
	return 0;

fail:
	if (isLocked) viUnlock(_io);
	return -1;
}


////////////////////////////////////////////////////////
// Get shutter transit delay
//   device: the shutter attached to the Arduino
//   transit time in ms
////////////////////////////////////////////////////////
int ARD_ShutterGetTransitDelay(int device, int *transDelay_ms)
{
	int isLocked=0;

	if (!_io) {
		reportError (__LINE__-2, __func__, "Device not open.");
		goto fail;
	}

	_status = viLock (_io, VI_EXCLUSIVE_LOCK, 5000, VI_NULL, VI_NULL);
	if(_status) {
		reportVisaError (__LINE__-2, __func__, _io, _status);
		goto fail;
	}
	isLocked=1;

	if (getDeviceParameterInt(_io, "TD", device, transDelay_ms)) {
		reportError (__LINE__-1, __func__, "Could not get shutter transit delay.");
		goto fail;
	}
	if (*transDelay_ms<0) {
		reportError (__LINE__-2, __func__, "Invalid transit delay.");
		goto fail;
	}

	_status = viUnlock (_io);
	if(_status) {
		reportVisaError (__LINE__-2, __func__, _io, _status);
		goto fail;
	}
	
	return 0;

fail:
	if (isLocked) viUnlock(_io);
	return -1;
}


////////////////////////////////////////////////////////
// Set shutter state
//   device: the shutter attached to the Arduino
//   state: 0->Closed, 1->Open
////////////////////////////////////////////////////////
int ARD_ShutterSetState(int device, int state)
{
	int isLocked=0;

	if (!_io) {
		reportError (__LINE__-2, __func__, "Device not open.");
		goto fail;
	}

	_status = viLock (_io, VI_EXCLUSIVE_LOCK, 5000, VI_NULL, VI_NULL);
	if(_status) {
		reportVisaError (__LINE__-2, __func__, _io, _status);
		goto fail;
	}
	isLocked=1;

	// set the shutter state
	if (state <0 || state > 1) {
		reportError (__LINE__-2, __func__, "Invalid state (0->Closed, 1->Open).");
		goto fail;
	}
	if (setDeviceParameterInt(_io, "ST", device, state)) {
		reportError (__LINE__-1, __func__, "Could not set shutter state.");
		goto fail;
	}

	_status = viUnlock (_io);
	if(_status) {
		reportVisaError (__LINE__-2, __func__, _io, _status);
		goto fail;
	}
	
	return 0;

fail:
	if (isLocked) viUnlock(_io);
	return -1;
}
	

////////////////////////////////////////////////////////
// Set shutter position
//   device: the shutter attached to the Arduino
//   position: actuator position
////////////////////////////////////////////////////////
int ARD_ShutterSetPosition(int device, int pos)
{
	int isLocked=0;

	if (!_io) {
		reportError (__LINE__-2, __func__, "Device not open.");
		goto fail;
	}

	_status = viLock (_io, VI_EXCLUSIVE_LOCK, 5000, VI_NULL, VI_NULL);
	if(_status) {
		reportVisaError (__LINE__-2, __func__, _io, _status);
		goto fail;
	}
	isLocked=1;

	// set the shutter position
	if (pos <0) {
		reportError (__LINE__-2, __func__, "Invalid position.");
		goto fail;
	}
	if (setDeviceParameterInt(_io, "SP", device, pos)) {
		reportError (__LINE__-1, __func__, "Could not set shutter position.");
		goto fail;
	}

	_status = viUnlock (_io);
	if(_status) {
		reportVisaError (__LINE__-2, __func__, _io, _status);
		goto fail;
	}
	
	return 0;

fail:
	if (isLocked) viUnlock(_io);
	return -1;
}
	

////////////////////////////////////////////////////////
// Get shutter parameters
//   device: the shutter attached to the Arduino
//   shieldChannel: actuator chanel on the shield
//   digIn: the number of the digital input (-1 for none)
//   open/closedPos: actuator value for the respective positions
//   transitDelay_ms: time for the shutter to open/close in ms
//   label: label to display
////////////////////////////////////////////////////////
int ARD_ShutterGetParameters(int device, int *shieldChannel, int *digIn, int *openPos, int *closedPos, int *transitDelay_ms, char* label)
{
	unsigned char instrResp[256];
	ViUInt32 charsRead;
	int respDev;
	int isLocked=0;

	if (!_io) {
		reportError (__LINE__-2, __func__, "Device not open.");
		goto fail;
	}

		_status = viLock (_io, VI_EXCLUSIVE_LOCK, 5000, VI_NULL, VI_NULL);
	if(_status) {
		reportVisaError (__LINE__-2, __func__, _io, _status);
		goto fail;
	}
	isLocked=1;

_status = viPrintf(_io, "GPR%d\n", device);
	if(_status) {
		reportVisaError (__LINE__-2, __func__, _io, _status);
		goto fail;
	}
	_status = viRead (_io, instrResp, 256, &charsRead);
	if(_status) {
		reportVisaError (__LINE__-2, __func__, _io, _status);
		goto fail;
	}
	if (charsRead<5) { // at least it should return "XX=d"
		reportError (__LINE__-1, __func__, "No command response received.");
		goto fail;
	}
	// check for response
	if (strnicmp((char *)instrResp, "Error:", 6)==0){
		instrResp[charsRead-2]='\0';	
		reportARDError (__LINE__-2, __func__, (char *)instrResp);
		goto fail;
	}
	if ( sscanf ((char *)instrResp, "PR%d,%d,%d,%d,%d,%d,%s", &respDev, shieldChannel, digIn, openPos, closedPos, transitDelay_ms, label) != 7) {
		reportError (__LINE__-1, __func__, "Could not read shutter return string.");
		goto fail;
	}
	if (respDev!=device) {
		reportError (__LINE__-1, __func__, "Responded with wrong device number.");
		goto fail;
	}
	
	_status = viUnlock (_io);
	if(_status) {
		reportVisaError (__LINE__-2, __func__, _io, _status);
		goto fail;
	}
	
	return 0;

fail:
	if (isLocked) viUnlock(_io);
	return -1;
}
	

////////////////////////////////////////////////////////
// Set shutter parameters
//   device: the shutter attached to the Arduino
//   shieldChannel: chanel on the shield
//   digIn: the number of the digital input (-1 for none)
//   open/closedPos: actuator value for the respective positions
//   transitDelay_ms: time for the shutter to open/close in ms
//   label: label to display
////////////////////////////////////////////////////////
int ARD_ShutterSetParameters(int device, int shieldChannel, int digInput, int openPos, int closedPos, int transitDelay_ms, const char* label)
{
	int isLocked=0;

	if (!_io) {
		reportError (__LINE__-2, __func__, "Device not open.");
		goto fail;
	}

		_status = viLock (_io, VI_EXCLUSIVE_LOCK, 5000, VI_NULL, VI_NULL);
	if(_status) {
		reportVisaError (__LINE__-2, __func__, _io, _status);
		goto fail;
	}
	isLocked=1;

	_status = viPrintf(_io, "SPR%d,%d,%d,%d,%d,%d,%s\n", device, shieldChannel, digInput, openPos, closedPos, transitDelay_ms, label);
	if(_status) {
		reportVisaError (__LINE__-2, __func__, _io, _status);
		goto fail;
	}
	if(checkErrorResponse(_io)!=0) {
		reportError (__LINE__-1, __func__, "ARD error:");
		goto fail;
	}
	
	_status = viUnlock (_io);
	if(_status) {
		reportVisaError (__LINE__-2, __func__, _io, _status);
		goto fail;
	}
	
	return 0;

fail:
	if (isLocked) viUnlock(_io);
	return -1;
}
	

////////////////////////////////////////////////////////
// Save parameters to EEPROM
////////////////////////////////////////////////////////
int ARD_ShutterSaveToEEPROM(void)
{
	int isLocked=0;

	if (!_io) {
		reportError (__LINE__-2, __func__, "Device not open.");
		goto fail;
	}

		_status = viLock (_io, VI_EXCLUSIVE_LOCK, 5000, VI_NULL, VI_NULL);
	if(_status) {
		reportVisaError (__LINE__-2, __func__, _io, _status);
		goto fail;
	}
	isLocked=1;

	_status = viPrintf(_io, "SAV\n");
	if(_status) {
		reportVisaError (__LINE__-2, __func__, _io, _status);
		goto fail;
	}
	if(checkErrorResponse(_io)!=0) {
		reportError (__LINE__-1, __func__, "ARD error:");
		goto fail;
	}

	_status = viUnlock (_io);
	if(_status) {
		reportVisaError (__LINE__-2, __func__, _io, _status);
		goto fail;
	}
	
	return 0;

fail:
	if (isLocked) viUnlock(_io);
	return -1;
}
	

// *****************************************************************************************
// Internal (non-exported) functions
// *****************************************************************************************
////////////////////////////////////////////////////////
// Get integer device parameter
////////////////////////////////////////////////////////
static int getDeviceParameterInt(ViSession io, const char *cmd, int device, int *param)
{
	unsigned char instrResp[256];
	ViUInt32 charsRead;
	char respCmd[3];
	int respDev, respParam;

	_status = viPrintf(io, "G%s%d\n", cmd, device);
	if(_status) {
		reportVisaError (__LINE__-2, __func__, io, _status);
		goto fail;
	}
	_status = viRead (io, instrResp, 256, &charsRead);
	if(_status) {
		reportVisaError (__LINE__-2, __func__, io, _status);
		goto fail;
	}
	if (charsRead<5) { // at least it should return "XX=d"
		reportError (__LINE__-2, __func__, "No command response received.");
		goto fail;
	}
	// check for response
	if (strnicmp((char *)instrResp, "Error:", 6)==0){
		instrResp[charsRead-2]='\0';	
		reportARDError (__LINE__-2, __func__, (char *)instrResp);
		goto fail;
	}
	if ( sscanf ((char *)instrResp, "%2s%d=%d", respCmd, &respDev, &respParam) != 3) {
		reportError (__LINE__-2, __func__, "Could not read shutter parameter.");
		goto fail;
	}
	if (strnicmp(respCmd, cmd, 2)!=0) {
		reportError (__LINE__-2, __func__, "Responded with wrong cmd identifier.");
		goto fail;
	}
	if (respDev!=device) {
		reportError (__LINE__-2, __func__, "Responded with wrong device number.");
		goto fail;
	}
	*param = respParam;

	return 0;
fail:
	return -1;
}


////////////////////////////////////////////////////////
// Get string device parameter
////////////////////////////////////////////////////////
static int getDeviceParameterString(ViSession io, const char *cmd, int device, char *str)
{
	unsigned char instrResp[256];
	ViUInt32 charsRead;
	char respCmd[3];
	int respDev;

	_status = viPrintf(io, "G%s%d\n", cmd, device);
	if(_status) {
		reportVisaError (__LINE__-2, __func__, io, _status);
		goto fail;
	}
	_status = viRead (io, instrResp, 256, &charsRead);
	if(_status) {
		reportVisaError (__LINE__-2, __func__, io, _status);
		goto fail;
	}
	if (charsRead<5) { // at least it should return "XX=d"
		reportError (__LINE__-2, __func__, "No command response received.");
		goto fail;
	}
	// check for response
	if (strnicmp((char *)instrResp, "Error:", 6)==0){
		instrResp[charsRead-2]='\0';	
		reportARDError (__LINE__-2, __func__, (char *)instrResp);
		goto fail;
	}
	if ( sscanf ((char *)instrResp, "%2s%d=%s", respCmd, &respDev, str) != 3) {
		reportError (__LINE__-2, __func__, "Could not read shutter return string.");
		goto fail;
	}
	if (strnicmp(respCmd, cmd, 2)!=0) {
		reportError (__LINE__-2, __func__, "Responded with wrong cmd identifier.");
		goto fail;
	}
	if (respDev!=device) {
		reportError (__LINE__-2, __func__, "Responded with wrong device number.");
		goto fail;
	}
		
	return 0;
fail:
	return -1;
}


////////////////////////////////////////////////////////
// Set int parameter
//   device: the shutter attached to the Arduino
////////////////////////////////////////////////////////
static int setDeviceParameterInt(ViSession io, const char *cmd, int device, int param)
{
	_status = viPrintf(io, "S%s%d,%d\n", cmd, device, param);
	if(_status) {
		reportVisaError (__LINE__-2, __func__, io, _status);
		goto fail;
	}
	if(checkErrorResponse(io)!=0) {
		reportError (__LINE__-1, __func__, "ARD error:");
		goto fail;
	}
	
	return 0;
fail:
	return -1;
}


////////////////////////////////////////////////////////
// Check for an OK response from the device
////////////////////////////////////////////////////////
static int checkErrorResponse(ViSession io)
{
	unsigned char instrResp[256];
	ViUInt32 charsRead;	

	_status = viRead (io, instrResp, 256, &charsRead);
	if(_status) {
		reportVisaError (__LINE__-2, __func__, io, _status);
		return -1;
	}
	// check for errors
	if ( strncmp ((char *)instrResp, "OK", 2) != 0) {
		instrResp[charsRead-2]='\0';	
		reportARDError (__LINE__-2, __func__, (char *)instrResp);
		return -1;
	}

	return 0;
}


////////////////////////////////////////////////////////
// Report a generic error within this module
////////////////////////////////////////////////////////
static void reportError(int line, const char* function, char* description )
{
	if ((line!=0) && (function!=NULL))
		printf("\nError in function %s (line %i of file %s): %s\n", function, line, __FILE__, description );
	else 
		printf("%s\n", description );
}

////////////////////////////////////////////////////////
// Report a VISA-generated error within this module
////////////////////////////////////////////////////////
static void reportVisaError(int line, const char* function, ViSession instr, ViStatus errStatus )
{
	char desc[256];

	_status = viStatusDesc (instr, errStatus, desc);
	if (!_status)
		printf("\nVisa error in function %s (line %i of file %s): %s\n", function, line, __FILE__, desc );
	else
		printf("\nVisa error in function %s (line %i of file %s), but could not get function description.\n",
					 function, line, __FILE__);
}

////////////////////////////////////////////////////////
// Report an Arduino-generated error within this module
////////////////////////////////////////////////////////
static void reportARDError(int line, const char* function, char* description )
{
	printf("\nArduino returned error in function %s (line %i of file %s): %s\n", function, line, __FILE__, description );
}

