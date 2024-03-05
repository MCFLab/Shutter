#include "Common.h"
// only include if DISPLAY_TFT is defined in "Common.h"
#ifdef DISPLAY_TFT


#include <Arduino.h>
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <Adafruit_FT6206.h>
#include "TFT.h"

#define SERIAL_DEBUG  0


// *************************************************************************************
// defines
// *************************************************************************************
// The display also uses hardware SPI, plus #9 & #10
// For the Adafruit shield, these are the default.
#define TFT_DC 9
#define TFT_CS 10
#define BACKLIGHTPIN 5

#define BLACK    0x0000
#define BLUE     0x001F
#define RED      0xF800
#define GREEN    0x07E0
#define CYAN     0x07FF
#define MAGENTA  0xF81F
#define YELLOW   0xFFE0 
#define WHITE    0xFFFF

#define SCREENROTATION  1 // (1: USB conn on left; 3: USB conn on right)


// *************************************************************************************
// global variables
// *************************************************************************************
Adafruit_ILI9341 _tftDev = Adafruit_ILI9341(TFT_CS, TFT_DC);
Adafruit_FT6206 _ts = Adafruit_FT6206(); // FT6206 uses hardware I2C (SCL/SDA)


// *************************************************************************************
// TFT class
// *************************************************************************************
////////////////////////////
// Constructor
TFT::TFT(unsigned int dispTurnoffInterval_s) : dispTurnoffInterval_s{dispTurnoffInterval_s}
{
  // set backlight on 
  pinMode(BACKLIGHTPIN, OUTPUT);
  digitalWrite(BACKLIGHTPIN, HIGH);
}

////////////////////////////
// Destructor
TFT::~TFT(void)
{
  // is never really called, since I never really 'quit' the program. Just for completeness
  for (int8_t z=0; z<TFT_MAXROWS; z++) {
    delete tftRowArr[z];
  }
  delete tftRowArr;
}

////////////////////////////
// Initialize stuff
TFT::Begin()
{
#if SERIAL_DEBUG>0
  Serial.println(F("TFT Begin."));
#endif      

  _tftDev.begin();
  _tftDev.setRotation(SCREENROTATION);
  _tftDev.fillScreen(ILI9341_BLACK);

  if (!_ts.begin(40)) { // set sensitivity coefficient
    Serial.println(F("Unable to start touchscreen."));
  } 
  else { 
#if SERIAL_DEBUG>0
    Serial.println(F("Touchscreen started.")); 
#endif
  }

  // allocate the rows
  tftRowArr = new TFTRow*[TFT_MAXROWS];
  for (int8_t z=0; z<TFT_MAXROWS; z++) {
    tftRowArr[z] = new TFTRow(z);
  }
}

////////////////////////////
// Set the members
TFT::SetNumDevs(int8_t numShutters)
{
  numRows = numShutters;
}
TFT::SetDevText(int8_t dev, char *label)
{
  tftRowArr[dev]->textElem->SetText(label);
}

////////////////////////////
// Refresh the entire display
TFT::RefreshDisplay()
{
  _tftDev.fillScreen(ILI9341_BLACK);
  for (int8_t z=0; z<numRows; z++)
  {
    RefreshDev(z);
  }
}

////////////////////////////
// Refresh only one dev
TFT::RefreshDev(int8_t dev)
{
  tftRowArr[dev]->openBut->Draw();
  tftRowArr[dev]->textElem->Draw();
  tftRowArr[dev]->closeBut->Draw();
}

////////////////////////////
// Set one line to on (1), off (0), or undefined (-1)
TFT::ChangeDevState(int8_t row, int8_t state)
{
  tftRowArr[row]->SetState(state);
  tftRowArr[row]->openBut->Draw();
  tftRowArr[row]->closeBut->Draw();
}

////////////////////////////
// Check for input
// return 1 if it was a valid request
// returns the device that requests the changed state by reference
int8_t TFT::CheckInput(int8_t *device, int8_t *state)
{
  unsigned long currentTime = millis();

  // set invalid defaults
  *device = -1;
  *state = -1;

  if (currentTime - lastTouchTime > TFT_BLOCKING_TIME_MS) {
    if (isDisplayOff) {
      if(_ts.touched()) {
        Wakeup();
        lastTouchTime = currentTime;
        isDisplayOff = 0;
      }
    } else { // display is still on
      if ( dispTurnoffInterval_s > 0
            && currentTime - lastTouchTime > 1000*(unsigned long) dispTurnoffInterval_s ) {
        Sleep();
        isDisplayOff = 1;
      } else { // check for button input
        if (_ts.touched()) {
          lastTouchTime = currentTime;
          uint16_t x, y;
          GetTouchCoordinates(&x, &y);
          int16_t height = _tftDev.height();
          int8_t row = (int8_t) (y / (height/TFT_MAXROWS)); 
          if (row>=0 && row < numRows)
            *device = row;
          else {
#if SERIAL_DEBUG>0
            Serial.println(F("Invalid touch row."));
#endif
            return 0;
          }
          if (tftRowArr[*device]->HasRequestedChange(x, y, state)) {
#if SERIAL_DEBUG>0
            Serial.print("Device "); Serial.print(*device); Serial.print(" requested state "); Serial.println(*state);
#endif
            return 1;
          }
        }
      }
    }
  }
  return 0;
}

////////////////////////////
// Get the touch point and scale to display
TFT::GetTouchCoordinates(uint16_t *x, uint16_t *y)
{
  TS_Point p = _ts.getPoint();
  // rotate coordinates
  int16_t height = _tftDev.height();
#if SCREENROTATION == 1
  int16_t width = _tftDev.width();
  *x = width - p.y;
  *y =  p.x;
#elif SCREENROTATION == 3
  *x = p.y;
  *y = height - p.x;
#else
  #error Invalid SCREENROTATION value (only 1 or 3 allowed)
#endif
}

////////////////////////////
// Put the display to sleep
TFT::Sleep()
{
  _tftDev.startWrite();
  _tftDev.writeCommand(ILI9341_DISPOFF);
  _tftDev.writeCommand(ILI9341_SLPIN);
  delay(120); // as required by TFT
  _tftDev.endWrite();
  digitalWrite(BACKLIGHTPIN, LOW);
}

////////////////////////////
// Wake up the display
TFT::Wakeup()
{
  _tftDev.startWrite();
  _tftDev.writeCommand(ILI9341_SLPOUT);
  delay(5); // as required by TFT
  _tftDev.writeCommand(ILI9341_DISPON);
  _tftDev.endWrite(); 
  digitalWrite(BACKLIGHTPIN, HIGH);
}


// *************************************************************************************
// TFTElement class
// *************************************************************************************
////////////////////////////
// Constructor
TFTElement::TFTElement(int16_t X, int16_t Y, int16_t W, int16_t H, int16_t thick=1):
  originX(X), originY(Y), width(W), height(H), lineThickness(thick) {}

////////////////////////////
// Set the text
void TFTElement::SetText(const char* txt)
{
  sprintf(text, "%-"MAXLABELCHARS_STR"s", txt);
}

////////////////////////////
// Set the colors
void TFTElement::SetColor(uint16_t colText, uint16_t colLine, uint16_t colBG)
{
  colorText = colText;
  colorLine = colLine;
  colorBG = colBG;
}

////////////////////////////
// Draw (or update) the element
void TFTElement::Draw(void)
{
  // draw solid rect, then smaller rect to create thicker line
  _tftDev.fillRoundRect(originX, originY, width, height, 3, colorLine);
  _tftDev.fillRoundRect(originX+lineThickness, originY+lineThickness,
                         width-2*lineThickness, height-2*lineThickness, 3, colorBG);
  _tftDev.setCursor(originX+lineThickness+2, originY+lineThickness+8);
  _tftDev.setTextColor(colorText);
  _tftDev.setTextSize(4);
  _tftDev.print(text);
}

// *************************************************************************************
// TFTButton class
// *************************************************************************************
////////////////////////////
// Constructor
TFTButton::TFTButton(int16_t X, int16_t Y, int16_t W, int16_t H, int16_t thick=1):
  TFTElement(X, Y, W, H, thick), isActive(0) {}

////////////////////////////
// Check if a touch coordinate is within the element
int8_t TFTButton::IsCoordInElement(int16_t x, int16_t y)
{
  if (     x > originX 
        && x < originX+width 
        && y > originY
        && y < originY+height )
    return 1;
  else
    return 0;
}

////////////////////////////
// Set the button as active (inverted colors) 
void TFTButton::SetActive(void)
{
  if (isActive) return;
  // exchange BG and text color 
  uint16_t tempColor = colorBG;
  colorBG = colorText;
  colorText = tempColor;
  isActive = 1;
  Draw();
}

////////////////////////////
// Set the button as inactive (regular colors) 
void TFTButton::SetInactive(void)
{
  if (!isActive) return;
  // exchange BG and text color 
  uint16_t tempColor = colorBG;
  colorBG = colorText;
  colorText = tempColor;
  isActive = 0;
  Draw();
}


// *************************************************************************************
// TFTRow class
// *************************************************************************************
////////////////////////////
// Constructor
TFTRow::TFTRow(int8_t row)
{
  int16_t w, h, rowHeight;

  w = _tftDev.width();
  h = _tftDev.height();
  rowHeight = h/TFT_MAXROWS;

  // allocate the elements
  int origY = row*rowHeight;
  int buttonWidth = rowHeight-TFT_BORDERWIDTH;

  openBut = new TFTButton(TFT_BORDERWIDTH/2, origY+TFT_BORDERWIDTH/2, buttonWidth, buttonWidth, 3);
  openBut->SetColor(GREEN, GREEN, ILI9341_BLACK);
  openBut->SetText("Op");

  textElem = new TFTElement(buttonWidth+3*TFT_BORDERWIDTH/2, origY+TFT_BORDERWIDTH/2,
                             w-3*TFT_BORDERWIDTH-2*buttonWidth, buttonWidth, 3);
  textElem->SetColor(YELLOW, YELLOW, ILI9341_BLACK);

  closeBut = new TFTButton(w-buttonWidth-TFT_BORDERWIDTH/2, origY+TFT_BORDERWIDTH/2,
                           buttonWidth, buttonWidth, 3);
  closeBut->SetColor(RED, RED, ILI9341_BLACK);
  closeBut->SetText("Cl");
}

////////////////////////////
// Destructor
TFTRow::~TFTRow()
{
  delete openBut;
  delete textElem;
  delete closeBut;
}

////////////////////////////
// Check if this specific row has requested a change in state
// returns 1 if a change occurred
// returns the requested state by reference
int8_t TFTRow::HasRequestedChange(int16_t x, int16_t y, int8_t *reqState)
{
  if( openBut->IsCoordInElement(x, y) ) {
    *reqState = 1;
    return 1;
  }
  if( closeBut->IsCoordInElement(x, y) ) {
    *reqState = 0;
    return 1;
  }
  return 0;
}

////////////////////////////
// Set the state programmatically (not by touch)
int8_t TFTRow::SetState(int8_t devState)
{
  if (devState==1) { // open 
    openBut->SetActive();
    closeBut->SetInactive();
  } else if (devState==0) { // close 
    openBut->SetInactive();
    closeBut->SetActive();
    } else if (devState==-1) { // idle 
    openBut->SetInactive();
    closeBut->SetInactive();
  }
  return 0;
}

#endif // DISPLAY_TFT