/*
6U5 Digital Replica by retronics.no
EM34 Digital Replica by Martin Wagner DL2WAG
*/

#include <Arduino.h> 
#include <Arduino_GFX_Library.h> //https://github.com/moononournation/Arduino_GFX | GFX Library for Arduino@^1.4.7

#define PIN_BACKLIGHT PB0
#define PIN_DC PB11 
#define PIN_CS PA4  
#define PIN_RES PB2
// PIN_SCL A5
// PIN_SDA A7
#define GREEN_BRIGHT RGB565(0, 255, 80)
#define GREEN_MEDIUM RGB565(0, 180, 60)
#define GREEN_DARK RGB565(0, 80, 35)
#define CENTER_X 63
#define CENTER_Y 75
#define OUTER_RADIUS 52
#define INNER_RADIUS 24
#define PIN_AGC PB1
#define BRIGHTNESS 255 // Max 255

Arduino_DataBus *bus = new Arduino_HWSPI(PIN_DC, PIN_CS);
Arduino_GFX *gfx = new Arduino_GC9107(bus, PIN_RES, 0 /* rotation */, true /* IPS */, GC9107_TFTWIDTH, GC9107_TFTHEIGHT, 0, 0, 0, 0);
HardwareSerial uart2(PA3, PA2);
uint8_t oldHalfAngleBottom;
uint8_t oldHalfAngleTop;
double agcFiltered;
uint8_t fadeInCounter=0;
uint32_t fadeInLastChange=millis();

void drawLine(uint16_t angle, uint16_t lineColor);

void setup(void)
{
  analogWriteResolution(16);
  uart2.begin(115200);
  pinMode(PIN_AGC, INPUT);
  pinMode(PIN_BACKLIGHT, OUTPUT);
  digitalWrite(PIN_BACKLIGHT,LOW);
  uart2.println("Digital EM34 replacement.");

  //Display startup
  if (!gfx->begin())
  {
    uart2.println("gfx->begin() failed!");
    delay(1000);
    NVIC_SystemReset();
  }
  gfx->fillScreen(BLACK);
  //circle
  for(int16_t i = 0; i < 360; i++)
  {
    drawLine(i,GREEN_DARK);
  }
  //left
  for(int16_t i = 46; i < 134; i++)
  {
    drawLine(i,GREEN_MEDIUM);
  }
  drawLine(45, GREEN_BRIGHT);
  drawLine(135, GREEN_BRIGHT);
  //right
  for(int16_t i = 226; i < 314; i++)
  {
    drawLine(i,GREEN_MEDIUM);
  }
  drawLine(225, GREEN_BRIGHT);
  drawLine(315, GREEN_BRIGHT);

  delay(1000);
}

void loop()
{
  //Fade in
  if((fadeInCounter<BRIGHTNESS)&&(millis()-fadeInLastChange)>(120-(fadeInCounter/4)))
  {
    fadeInCounter++;
    analogWrite(PIN_BACKLIGHT, fadeInCounter*fadeInCounter);
    fadeInLastChange=millis();
  }
  //AGC measurement, filtering and mapping to display angle //todo modify for em34
  uint16_t agcRaw=analogRead(PIN_AGC);
  agcFiltered=(9.0*agcFiltered+agcRaw)/10.0;
  uint8_t newHalfAngleBottom=(uint8_t)(((agcFiltered/11)+15.0*log10(1+agcFiltered))/3);
  uint8_t newHalfAngleTop = (uint8_t)((((agcFiltered * 4)/11)+15.0*log10(1+(agcFiltered * 4)))/3);
  //crop
  newHalfAngleBottom=newHalfAngleBottom>45?45:newHalfAngleBottom;
  newHalfAngleTop=newHalfAngleTop>45?45:newHalfAngleTop;

  //todo remove
  uart2.println("Raw: " + String(agcFiltered) + ", Volt: " + String(agcFiltered / 1024 * 22.5) + ", Angle Bot: " + String(newHalfAngleBottom) + ", Angle Top: " + String(newHalfAngleTop));

  //Display update
  if(newHalfAngleTop>oldHalfAngleTop)
  {
    for(int8_t i=oldHalfAngleTop; i<newHalfAngleTop; i++)
    {
      drawLine(135+i,GREEN_MEDIUM);
      drawLine(225-i,GREEN_MEDIUM);
    }
    drawLine(135+newHalfAngleTop,GREEN_BRIGHT);
    drawLine(225-newHalfAngleTop,GREEN_BRIGHT);
  }
  if(newHalfAngleTop<oldHalfAngleTop)
  {
    for(int8_t i=oldHalfAngleTop; i>newHalfAngleTop; i--)
    {
      drawLine(135+i,GREEN_DARK);
      drawLine(225-i,GREEN_DARK);
    }
    drawLine(135+newHalfAngleTop,GREEN_BRIGHT);
    drawLine(225-newHalfAngleTop,GREEN_BRIGHT);
  }
  if(newHalfAngleBottom>oldHalfAngleBottom)
  {
    for(int8_t i=oldHalfAngleBottom; i<newHalfAngleBottom; i++)
    {
      drawLine(315+i,GREEN_MEDIUM);
      drawLine(405-i,GREEN_MEDIUM);
    }
    drawLine(315+newHalfAngleBottom,GREEN_BRIGHT);
    drawLine(405-newHalfAngleBottom,GREEN_BRIGHT);
  }
  if(newHalfAngleBottom<oldHalfAngleBottom)
  {
    for(int8_t i=oldHalfAngleBottom; i>newHalfAngleBottom; i--)
    {
      drawLine(315+i,GREEN_DARK);
      drawLine(405-i,GREEN_DARK);
    }
    drawLine(315+newHalfAngleBottom,GREEN_BRIGHT);
    drawLine(405-newHalfAngleBottom,GREEN_BRIGHT);
  }
  oldHalfAngleBottom=newHalfAngleBottom;
  oldHalfAngleTop=newHalfAngleTop;
}


void drawLine(uint16_t angle, uint16_t lineColor)
{
  double sinVal = sin(angle*0.0174533);
  double cosVal = cos(angle*0.0174533);
  gfx->drawLine(CENTER_X+sinVal*INNER_RADIUS, CENTER_Y+cosVal*INNER_RADIUS, CENTER_X+sinVal*OUTER_RADIUS, CENTER_Y+cosVal*OUTER_RADIUS, lineColor);
  gfx->drawLine(CENTER_X+sinVal*INNER_RADIUS, CENTER_Y+1+cosVal*INNER_RADIUS, CENTER_X+sinVal*OUTER_RADIUS, CENTER_Y+1+cosVal*OUTER_RADIUS, lineColor);
}
