#include "Common.h"
// only include if DISPLAY_TFT is defined in "Common.h"
#ifdef DISPLAY_TFT


#ifndef TFT_H
#define TFT_H

// *************************************************************************************
// TFTElement class
// *************************************************************************************
class TFTElement
{
protected:
  int16_t originX, originY;
  int16_t width, height;
  char text[MAXLABELCHARS+1];
  uint16_t colorText, colorLine, colorBG;
  int16_t lineThickness;
public:
  TFTElement(int16_t X, int16_t Y, int16_t W, int16_t H, int16_t thick);
  void SetText(const char* txt);
  void SetColor(uint16_t colText, uint16_t colLine, uint16_t colBG);
  void Draw(void);
};

// *************************************************************************************
// TFTButton class
// *************************************************************************************
class TFTButton: public TFTElement
{
  int8_t isActive;
public:
  TFTButton(int16_t X, int16_t Y, int16_t W, int16_t H, int16_t thick);
  int8_t IsCoordInElement(int16_t x, int16_t y);
  void SetActive(void);
  void SetInactive(void);
};

// *************************************************************************************
// TFTRow class
// *************************************************************************************
class TFTRow
{
public:
  TFTButton *openBut;
  TFTElement *textElem;
  TFTButton *closeBut;
  TFTRow(int8_t row);
  ~TFTRow();
  int8_t HasRequestedChange(int16_t x, int16_t y, int8_t *reqState);
  int8_t SetState(int8_t state);
};

// *************************************************************************************
// TFT class
// *************************************************************************************
class TFT
{
private:
  TFTRow **tftRowArr;
  int8_t numRows = 0;
  unsigned long lastTouchTime = 0;
  unsigned int dispTurnoffInterval_s;
  int8_t isDisplayOff = 0;
  GetTouchCoordinates(uint16_t *x, uint16_t *y);
  Sleep();
  Wakeup();

public:
  TFT(unsigned int dispTurnoffInterval_s = TFT_DIM_PERIOD_S);
  ~TFT();
  Begin();
  SetNumDevs(int8_t numShutters);
  SetDevText(int8_t dev, char *label);
  RefreshDisplay();
  RefreshDev(int8_t dev);
  ChangeDevState(int8_t dev, int8_t state);
  int8_t CheckInput(int8_t *device, int8_t *state);
};

#endif // TFT_H

#endif // DISPLAY_TFT
