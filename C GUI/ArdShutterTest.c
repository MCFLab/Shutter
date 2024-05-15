// *****************************************************************************************
//
// Test panel source for the Arduino shutter function
//
// *****************************************************************************************


#include <ansi_c.h>
#include <formatio.h>
#include <cvirte.h>		
#include <userint.h>

#include "ArdShutter.h"
#include "ArdShutterTest.h"

// *****************************************************************************************
// Defines
// *****************************************************************************************
//#define ARD_SHUTTER_ADDR	"ASRL11::INSTR"


// *****************************************************************************************
// Internal function prototypes
// *****************************************************************************************
static void newTextLine (int panelHandle, int controlID, const char* text);
static void appendToTextLine (int panelHandle, int controlID, const char* text);


// *****************************************************************************************
// Global variables
// *****************************************************************************************
static int _mainPanel;
static int _status;

// *****************************************************************************************
// main
// *****************************************************************************************
int main (int argc, char *argv[])
{
	char buffer[100];
	char adrStr[100]; 
	
	if (InitCVIRTE (0, argv, 0) == 0)
		return -1;	/* out of memory */
	if ((_mainPanel = LoadPanel (0, "ArdShutterTest.uir", MAIN_PAN)) < 0)
		return -1;
	DisplayPanel (_mainPanel);

	// ask for instrument address
	PromptPopup ("Arduino Interface", "COM address", buffer, 99);
	sprintf(adrStr, "ASRL%s::INSTR",buffer);
	
	// opening connection to Arduino
	newTextLine (_mainPanel, MAIN_PAN_STATUS_BOX, "Opening connection to Arduino ... ");
//	_status = ARD_ShutterInit(ARD_SHUTTER_ADDR);
	_status = ARD_ShutterInit(adrStr);
	if (_status)
		appendToTextLine (_mainPanel, MAIN_PAN_STATUS_BOX, "failed.");
	else
		appendToTextLine (_mainPanel, MAIN_PAN_STATUS_BOX, "done.");
	
	RunUserInterface ();

	DiscardPanel (_mainPanel);
	return 0;
}



// *****************************************************************************************
// Main panel callbacks
// *****************************************************************************************

int CVICALLBACK quit (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:

			newTextLine (_mainPanel, MAIN_PAN_STATUS_BOX, "Closing Arduino ... ");
			_status = ARD_Close();

			QuitUserInterface (0);
			break;
		}
	return 0;
}


int CVICALLBACK setShutter (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int shutter;
	
	switch (event)
		{
		case EVENT_COMMIT:

			GetCtrlVal(panel, MAIN_PAN_SHUTTER_NUM, &shutter);
			switch (control) {
				case MAIN_PAN_CLOSE_BUT:
					_status = ARD_ShutterSetState(shutter, 0);
					break;
				case MAIN_PAN_OPEN_BUT:
					_status = ARD_ShutterSetState(shutter, 1);
					break;
			}
			break;
		}
	return 0;
}


int CVICALLBACK setManual (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int shutter;
	int pos;
	
	switch (event)
		{
		case EVENT_VAL_CHANGED:

			GetCtrlVal(panel, MAIN_PAN_SHUTTER_NUM, &shutter);
			GetCtrlVal(panel, control, &pos);
			_status = ARD_ShutterSetPosition(shutter, pos);

			break;
		}
	return 0;
}


int CVICALLBACK getShutterState (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int shutter;
	int state;
	int transDelay;
	char label[11];
	char line[100];
	
	switch (event)
		{
		case EVENT_COMMIT:

			GetCtrlVal(panel, MAIN_PAN_SHUTTER_NUM, &shutter);

			if((_status = ARD_ShutterGetDeviceLabel(shutter, label))) return 0;
			if((_status = ARD_ShutterGetState(shutter, &state))) return 0;
			if((_status = ARD_ShutterGetTransitDelay(shutter, &transDelay))) return 0;
			
			sprintf(line, "Shutter%d, Label=%s, TransDelay=%dms State=%d.", shutter, label, transDelay, state);
			newTextLine (_mainPanel, MAIN_PAN_STATUS_BOX, line);
			
			break;
		}
	return 0;
}

int CVICALLBACK getNumDev (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int numDev;
	
	switch (event)
		{
		case EVENT_COMMIT:

			if((_status = ARD_ShutterGetNumDevices(&numDev))) return 0;
			SetCtrlVal(panel, MAIN_PAN_NUMDEVICES_NUM, numDev);

			break;
		}
	return 0;
}


int CVICALLBACK clearDev (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:

			_status = ARD_ShutterClearDev();
			break;

		}
	return 0;
}


int CVICALLBACK param (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int shutter;
	int PWMChannel, digInput, openPos, closedPos, delay;
	char label[11];
	
	switch (event)
		{
		case EVENT_COMMIT:

			GetCtrlVal(panel, MAIN_PAN_SHUTTER_NUM, &shutter);

			switch(control) {
				case MAIN_PAN_GETPARAM_BUT:
					if((_status = ARD_ShutterGetParameters(shutter, &PWMChannel, &digInput, &openPos, &closedPos, &delay, label))) return 0;
					SetCtrlVal(panel, MAIN_PAN_SHIELDCH_NUM, PWMChannel);
					SetCtrlVal(panel, MAIN_PAN_DIGIN_NUM, digInput);
					SetCtrlVal(panel, MAIN_PAN_OPENPOS_NUM, openPos);
					SetCtrlVal(panel, MAIN_PAN_CLOSEDPOS_NUM, closedPos);
					SetCtrlVal(panel, MAIN_PAN_TRANSDELAY_NUM, delay);
					SetCtrlVal(panel, MAIN_PAN_LABEL_STR, label);
					break;
				case MAIN_PAN_SETPARAM_BUT:
					GetCtrlVal(panel, MAIN_PAN_SHIELDCH_NUM, &PWMChannel);
					GetCtrlVal(panel, MAIN_PAN_DIGIN_NUM, &digInput);
					GetCtrlVal(panel, MAIN_PAN_OPENPOS_NUM, &openPos);
					GetCtrlVal(panel, MAIN_PAN_CLOSEDPOS_NUM, &closedPos);
					GetCtrlVal(panel, MAIN_PAN_TRANSDELAY_NUM, &delay);
					GetCtrlVal(panel, MAIN_PAN_LABEL_STR, label);
					_status = ARD_ShutterSetParameters(shutter, PWMChannel, digInput, openPos, closedPos, delay, label);
					break;
			}
					
			break;
		}
	return 0;
}

int CVICALLBACK saveEeprom (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:

			_status = ARD_ShutterSaveToEEPROM();
			break;

		}
	return 0;
}


// *****************************************************************************************
// Internal functions
// *****************************************************************************************

////////////////////////////////////////////////////////
// adds a new text line to a TextBox and scolls if appropriate
////////////////////////////////////////////////////////
static void newTextLine (int panelHandle, int controlID, const char* text)
{
	int numTextLines;
	int visibleLines;

	InsertTextBoxLine (panelHandle, controlID, -1, text);
	ProcessDrawEvents ();
	GetNumTextBoxLines (panelHandle, controlID, &numTextLines);
	GetCtrlAttribute (panelHandle, controlID, ATTR_VISIBLE_LINES, &visibleLines);
	if (numTextLines-1>visibleLines)
		SetCtrlAttribute (panelHandle, controlID, ATTR_FIRST_VISIBLE_LINE,
								numTextLines-1-visibleLines);
	ProcessDrawEvents ();
}


////////////////////////////////////////////////////////
// appends text to the last line in a TextBox
////////////////////////////////////////////////////////
static void appendToTextLine (int panelHandle, int controlID, const char* text)
{
	int numTextLines;
	int length;
	char line[200];

	GetNumTextBoxLines (panelHandle, controlID, &numTextLines);
	GetTextBoxLine (panelHandle, controlID, numTextLines-2, line);
	GetTextBoxLineLength (panelHandle, controlID, numTextLines-2, &length);
	CopyString (line, length, text, 0, (ssize_t) strlen(text)+1);
	ReplaceTextBoxLine (panelHandle, controlID, numTextLines-2, line);
	ProcessDrawEvents ();
}

