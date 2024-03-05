/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/

#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif

     /* Panels and Controls: */

#define  MAIN_PAN                         1
#define  MAIN_PAN_OPEN_BUT                2       /* control type: command, callback function: setShutter */
#define  MAIN_PAN_GETPARAM_BUT            3       /* control type: command, callback function: param */
#define  MAIN_PAN_SETPARAM_BUT            4       /* control type: command, callback function: param */
#define  MAIN_PAN_CLOSE_BUT               5       /* control type: command, callback function: setShutter */
#define  MAIN_PAN_SAVE_BUT                6       /* control type: command, callback function: saveEeprom */
#define  MAIN_PAN_CLEAR_BUT               7       /* control type: command, callback function: clearDev */
#define  MAIN_PAN_CHECKNUMDEV_BUT         8       /* control type: command, callback function: getNumDev */
#define  MAIN_PAN_STATUS_BUT              9       /* control type: command, callback function: getShutterState */
#define  MAIN_PAN_STATUS_BOX              10      /* control type: textBox, callback function: (none) */
#define  MAIN_PAN_QUIT_BUT                11      /* control type: command, callback function: quit */
#define  MAIN_PAN_TRANSDELAY_NUM          12      /* control type: numeric, callback function: (none) */
#define  MAIN_PAN_CLOSEDPOS_NUM           13      /* control type: numeric, callback function: (none) */
#define  MAIN_PAN_OPENPOS_NUM             14      /* control type: numeric, callback function: (none) */
#define  MAIN_PAN_PWMCH_NUM               15      /* control type: numeric, callback function: (none) */
#define  MAIN_PAN_MANUAL_NUM              16      /* control type: numeric, callback function: setManual */
#define  MAIN_PAN_NUMDEVICES_NUM          17      /* control type: numeric, callback function: (none) */
#define  MAIN_PAN_DIGIN_NUM               18      /* control type: numeric, callback function: (none) */
#define  MAIN_PAN_SHUTTER_NUM             19      /* control type: numeric, callback function: (none) */
#define  MAIN_PAN_LABEL_STR               20      /* control type: string, callback function: (none) */
#define  MAIN_PAN_TEXTMSG                 21      /* control type: textMsg, callback function: (none) */


     /* Control Arrays: */

          /* (no control arrays in the resource file) */


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */

int  CVICALLBACK clearDev(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK getNumDev(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK getShutterState(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK param(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK quit(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK saveEeprom(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK setManual(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK setShutter(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif