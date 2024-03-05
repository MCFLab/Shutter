#include "Common.h"
// only include if LCD is chosen in "Common.h"
#ifdef DISPLAY_LCD


#ifndef LCD_H
#define LCD_H

class LCD
{
private:
  char devLabels[MAXSHUTTERS][MAXLABELCHARS+1];
  int8_t numDevs = 0;
  int8_t currDevice = 0;
  int8_t devState[MAXSHUTTERS] = {-2};
  unsigned long lastButtonTime = 0;
  unsigned int dispTurnoffInterval_s;
  int8_t isDisplayOff = 0;
  Sleep();
  Wakeup();

public:
  LCD(unsigned int dispTurnoffInterval_s = LCD_DIM_PERIOD_S);
  Begin();
  SetNumDevs(int8_t numShutters);
  SetDevText(int8_t device, char *label);
  RefreshDisplay();
  RefreshDev(int8_t device);
  ChangeDevState(int8_t device, int8_t state);
  int8_t CheckInput(int8_t *device, int8_t *state);
};

#endif // LCD_H

#endif // DISPLAY_LCD