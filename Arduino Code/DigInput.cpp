#include "Common.h"
// only include if DIGINPUT is defined in "Common.h"
#ifdef DIGINPUT


#include <Arduino.h>
#include <avr/interrupt.h>
#include "DigInput.h"

#define SERIAL_DEBUG  0


// *************************************************************************************
// defines
// *************************************************************************************
// defines for the digital input
#define CTRL0   PCINT18 // pin for control line 0; PCINT18->Arduino pin D2
#define CTRL1   PCINT19 // pin for control line 1; PCINT19->Arduino pin D3
#define CTRL2   PCINT22 // pin for control line 2; PCINT22->Arduino pin D6
#define CTRL3   PCINT23 // pin for control line 3; PCINT23->Arduino pin D7
#define PCINT_MASK        (bit(CTRL0)|bit(CTRL1)|bit(CTRL2)|bit(CTRL3)) // mask for the PC interrupt
#define PIN_SETTLED       0   // bit in the status flag to indicate the end of bouncing


// *************************************************************************************
// global variables
// *************************************************************************************


// *************************************************************************************
// DigInput class
// *************************************************************************************
////////////////////////////
// Constructor
DigInput::DigInput(){}

////////////////////////////
// Initialize stuff
DigInput::Begin()
{
#if SERIAL_DEBUG>0
  Serial.println(F("DigInput Begin."));
#endif      

  // mode is either INPUT ot INPUT_PULLUP
  uint8_t mode = (DIGINPUT_USEPULLUPS==1 ? INPUT_PULLUP : INPUT);
  pinMode(CTRL0, mode); 
  pinMode(CTRL1, mode); 
  pinMode(CTRL2, mode); 
  pinMode(CTRL3, mode); 

  noInterrupts();
  // set up pin change interrupt
  PCICR |= bit(PCIE2);  // enable PCI on port 2 (port D, PCINT16 - PCINT23)
  PCMSK2 |= PCINT_MASK; // marks the pins to monitor
  // set up the timer1 interrupt
  TCCR1A = TCCR1B = 0;  // clear register
  TCNT1 = 0; // initialize counter value to 0
  TCCR1B |= bit(CS12) | bit(CS10); // set CS12 and CS10 bits for 1024 prescaler 
  OCR1A = (uint16_t)(CHECK_INTERVAL_MS*15.625 - 1); // set compare match register (must be <65536)
  TCCR1B |= bit(WGM12); // turn on CTC mode
  interrupts();

  DigInput::status = 0;
}

////////////////////////////
// check whether an input has changed
// returns 1 if something has changed, 0 otherwise
// pinstate contains the desired shutter configuration in bits 0-3
uint8_t DigInput::CheckState(uint8_t *pinState)
{
  uint8_t flag;
  uint8_t hs;

  // copy and clear status flag
  noInterrupts();
  flag = DigInput::status;
  DigInput::status = 0;
  hs = DigInput::highState;
  interrupts();

  if (flag & bit(PIN_SETTLED)) {
#if SERIAL_DEBUG>0
    Serial.print(F("State = ")); Serial.println(hs);
#endif      
    *pinState = 0;
    if (hs & bit(CTRL0)) *pinState |= bit(0);
    if (hs & bit(CTRL1)) *pinState |= bit(1);
    if (hs & bit(CTRL2)) *pinState |= bit(2);
    if (hs & bit(CTRL3)) *pinState |= bit(3);
//    *pinState = (hs>>4); // shift pins 4-7 to bits 0-3
#if SERIAL_DEBUG>0
    Serial.print(F("PinState = ")); Serial.println(*pinState);
#endif      
    return 1;
  } else {
    return 0;
  }
}

// **********************
// interrupt service routines
// **********************
////////////////////////////
// pin change interrupt gets called when any of the monitored pins change
ISR(PCINT2_vect){
  // stop PC interrupt
  PCICR &= ~bit(PCIE2);  // disable PCI on port 2
  // start timer -> this runs the timer for CHECK_INTERVAL_MS ms
  TCNT1 = 0; // reset the timer to zero
  TIMSK1 |= bit(OCIE1A); // enable timer compare interrupt

  // this ensures that the debounce check runs one full cycle
  DigInput::state[DigInput::index] = ~(PIND & PCINT_MASK);
  DigInput::index++;
  if(DigInput::index>=MAX_CHECKS) DigInput::index=0;

}

////////////////////////////
// timer interrupt gets called when count is met (every CHECK_INTERVAL_MS until stopped)
ISR(TIMER1_COMPA_vect)
{
  uint8_t z;
  uint8_t low, high;

  DigInput::state[DigInput::index] = PIND & PCINT_MASK; // read the port
  DigInput::index++;
  // check if the pins have settled (all the entries match, all pins high or all low)
  high = low = 0xFF;
  for (z=0; z<MAX_CHECKS;z++) {
    high = high & DigInput::state[z];  // check for all bits high
    low = low & ~DigInput::state[z];  // check for all bits low
  }
  DigInput::highState = high; // bits that are 1 have settled at high, all others have changed
  DigInput::lowState = low; // bits that are 1 have settled at low, all others have changed
  if(DigInput::index>=MAX_CHECKS) DigInput::index=0;

  if ( (high | low) == 0xFF ) { // all pins have settled either high or low
    // set the flag and stop the timer
    DigInput::status |= bit(PIN_SETTLED); // set the flag that the pins have finished bouncing
    // stop timer
    TIMSK1 &= ~bit(OCIE1A); // disable timer compare interrupt
    // start PC interrupt
    PCIFR = bit(PCIF2);  // clear the interrupt flag
    PCICR |= bit(PCIE2);  // enable PCI on port 2
  } // else: keep going with the timer until settled
}

#endif // DIGINPUT