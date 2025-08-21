/*
 * tube_6u5.cpp
 *
 *  Created on: Apr 6, 2025
 *      Author: martin
 */

#include "tube_6u5.h"

Tube6u5::Tube6u5(Color color, int32_t pinBacklightPwm, int32_t pinEnable,
    HardwareSerial &uart, Arduino_GFX *gfx, bool print) :
    Eyetube(color, pinBacklightPwm, pinEnable, uart, gfx, print)
{

  //circle
  for(int16_t i = 46; i < 315; i++)
  {
    drawLine(i,cMedium);
  }
  drawLine(315, cBright);
  drawLine(45, cBright);
  //inactive area
  for(int16_t i = 316; i < 404; i++)
  {
    drawLine(i,cDark);
  }
}

void Tube6u5::run(uint16_t agcRaw, uint16_t brightness)
{
  //Enable
  if ((pinEnable < 0) || digitalRead(pinEnable)) {
    analogWrite(pinBacklightPwm, brightness);
  } else {
    analogWrite(pinBacklightPwm, 0);
  }

  //AGC measurement, filtering and mapping to display angle
  agcFiltered=(9.0*agcFiltered+agcRaw)/10.0;
  uint8_t newHalfAngle=(uint8_t)(((agcFiltered/11)+15.0*log10(1+agcFiltered))/3);
  newHalfAngle=newHalfAngle>45?45:newHalfAngle;

  //Display update
  if(newHalfAngle>oldHalfAngle)
  {
    for(int8_t i=oldHalfAngle; i<newHalfAngle; i++)
    {
      drawLine(315+i,cMedium);
      drawLine(405-i,cMedium);
    }
    drawLine(315+newHalfAngle,cBright);
    drawLine(405-newHalfAngle,cBright);
  }
  else if(newHalfAngle<oldHalfAngle)
  {
    for(int8_t i=oldHalfAngle; i>newHalfAngle; i--)
    {
      drawLine(315+i,cDark);
      drawLine(405-i,cDark);
    }
    drawLine(315+newHalfAngle,cBright);
    drawLine(405-newHalfAngle,cBright);
  }
  oldHalfAngle=newHalfAngle;
}

