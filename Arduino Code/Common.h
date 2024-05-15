#ifndef COMMON_H
#define COMMON_H

//////////////
// devices/functionality to use (comment if unused)
#define SERIALCOMM

// pick at most one of the displays:
//#define DISPLAY_LCD
#define DISPLAY_TFT

//#define DIGINPUT

// pick exactly one of the shutter types:
#define SHUTTER_RCSERVO
//#define SHUTTER_SOLENOID
//////////////

//////////////
// general definitions
#define ID_STRING "Arduino Uno Shutter 4.0"
#define MAXSHUTTERS 4 // max devices in the parameters class
#define IDLEINTERVAL_S 0 // time in s after which the servo disengages, zero for never
                         //   only use for servos, not for solenoids (leave at 0 then) 
//////////////

//////////////
// module-specific definitions
#define RCSERVO_BOARDID 0x40 // I2C address of PWM servo board
#define RCSERVO_FREQ 50 // update rate for the servo shield, analog servos run at ~50 Hz 

#define SOLENOID_BOARDID 0x60 // I2C address of motor board

#define SERIAL_BAUDRATE 9600
#define SERIAL_TERMCHAR 0xA  // can be 0xA (LF) or 0xD  (CR)

#define TFT_BORDERWIDTH 6 // width of the border around buttons (in px) 
#define TFT_BLOCKING_TIME_MS 100 // time in ms during which a new touch is ignored. Used for debouncing
// for TFT: easiest if TFT_MAXROWS = MAXSHUTTERS
#define TFT_MAXROWS 4 // number of rows on the TFT
#define TFT_DIM_PERIOD_S 60 // time in s after which display dims
#define TFT_SCREENROTATION  1 // (1: USB conn on left; 3: USB conn on right)

#define LCD_BLOCKING_TIME_MS 300 // time in ms during which a new keypress is ignored. Used for debouncing
#define LCD_DIM_PERIOD_S 60 // time in s after which display dims

// size of the label string (common for TFT and LCD) 
// For TFT pick 7 - can do more if font is made smaller
// For LCD pick 16 (width of the display) 
#define MAXLABELCHARS 7
#define MAXLABELCHARS_STR "7" // (same number, but as a string for scanf/printf formatting)

#define DIGINPUT_USEPULLUPS 0 // 1->use internal Arduino pullup resistors, 0 if not
// the total debounce period for the digital inputs is (DIGINPUT_MAX_CHECKS * DIGINPUT_CHECK_INTERVAL_MS) in ms
// if not debouncing is required, set DIGINPUT_CHECK_INTERVAL_MS to zero (DIGINPUT_MAX_CHECKS must be >0)
#define DIGINPUT_CHECK_INTERVAL_MS 0   // interval in ms for the bounce check
#define DIGINPUT_MAX_CHECKS        10  // number of intervals in a bounce check cycle
//////////////

#endif