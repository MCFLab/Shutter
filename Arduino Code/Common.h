#ifndef COMMON_H
#define COMMON_H

//////////////
// devices/functionality to use (comment if unused)
#define SERIALCOMM

// pick one, but not both of the displays:
//#define DISPLAY_LCD
#define DISPLAY_TFT

//#define DIGINPUT
//////////////

#define ID_STRING "Arduino Uno Shutter 4.0"
#define MAXSHUTTERS 4 // max devices in the parameters class
#define IDLEINTERVAL_S 10 //  time in s after which the servo disengages, zero for never

#define SERVO_FREQ 50 // update rate for the servo shield, analog servos run at ~50 Hz 

#define SERIAL_BAUDRATE 9600
#define SERIAL_TERMCHAR 0xA  // LF
//#define SERIAL_TERMCHAR 0xD  // CR

#define TFT_BORDERWIDTH 6 // width of the border around buttons (in px) 
#define TFT_BLOCKING_TIME_MS 100 // time in ms during which a new touch is ignored. Used for debouncing
// for TFT: easiest if TFT_MAXROWS = MAXSHUTTERS
#define TFT_MAXROWS 4 // number of rows on the TFT
#define TFT_DIM_PERIOD_S 60 // time in s after which display dims

#define LCD_BLOCKING_TIME_MS 300 // time in ms during which a new keypress is ignored. Used for debouncing
#define LCD_DIM_PERIOD_S 60 // time in s after which display dims


// size of the label string (common for TFT and LCD) 
// For TFT pick 7 - can do more if font is made smaller
// For LCD pick 16 (width of the display) 
#define MAXLABELCHARS 7
#define MAXLABELCHARS_STR "7" // (same number, but as a string for scanf/printf formatting)

#define DIGINPUT_USEPULLUPS 1 // 1->use internal Arduino pullup resistors, 0 if not

#endif