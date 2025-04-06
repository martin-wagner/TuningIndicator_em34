/*
 * tube_em34.cpp
 *
 *  Created on: Apr 6, 2025
 *      Author: martin
 */

#include "tube_em34.h"

TubeEm34::TubeEm34(Color color, int32_t pinBacklightPwm, int32_t pinEnable,
    HardwareSerial &uart, Arduino_GFX *gfx, bool print) :
    Eyetube(color, pinBacklightPwm, pinEnable, uart, gfx, print)
{
  //circle
  for (int16_t i = 0; i < 360; i++) {
    drawLine(i, GREEN_DARK);
  }
  //left
  for (int16_t i = 45; i < 135; i++) {
    drawLine(i, GREEN_MEDIUM);
  }
  drawLine(45, GREEN_BRIGHT);
  drawLine(135, GREEN_BRIGHT);
  //right
  for (int16_t i = 225; i < 315; i++) {
    drawLine(i, GREEN_MEDIUM);
  }
  drawLine(225, GREEN_BRIGHT);
  drawLine(315, GREEN_BRIGHT);
}

void TubeEm34::run(uint16_t agcRaw, uint16_t brightness)
{
  //Enable
  if ((pinEnable < 0) || digitalRead(pinEnable)) {
    analogWrite(pinBacklightPwm, brightness);
  } else {
    analogWrite(pinBacklightPwm, 0);
  }

  //AGC measurement, filtering and mapping to display angle
  //-0.00321774x^{3}+0.311724x^{2}-9.33456x+90.9922 (0 ... 17V) low sensitivity (bottom)
  //-0.184343x^{3}+4.50276x^{2}-35.8763x+94.963     (0...7V)    high sensivity (top)
  agcFiltered = (9.0 * agcFiltered + agcRaw) / 10.0;
  float x = agcFiltered / 1024 * 22.5; //agc voltage [V]
  uint8_t newHalfAngleBottom = 45;
  if (x < 17.0) {
    newHalfAngleBottom = (uint8_t) ((float) 45.0
        - (-0.00321774 * pow(x, 3) + 0.311724 * pow(x, 2) - 9.33456 * x + 89)
            / 2);
  }
  uint8_t newHalfAngleTop = 45;
  if (x < 7.0) {
    newHalfAngleTop = (uint8_t) ((float) 45.0
        - (-0.184343 * pow(x, 3) + 4.50276 * pow(x, 2) - 35.8763 * x + 94.963)
            / 2);
  }
  //crop
  newHalfAngleBottom = newHalfAngleBottom > 45 ? 45 : newHalfAngleBottom;
  newHalfAngleTop = newHalfAngleTop > 45 ? 45 : newHalfAngleTop;

  if (print) {
    uart.println(
        "Raw: " + String(agcFiltered) + ", Volt: " + String(x) + ", Angle Bot: "
            + String(newHalfAngleBottom) + ", Angle Top: "
            + String(newHalfAngleTop));
  }

  //Display update
  if (newHalfAngleTop > oldHalfAngleTop) {
    for (int8_t i = oldHalfAngleTop; i < newHalfAngleTop; i++) {
      drawLine(135 + i, GREEN_MEDIUM);
      drawLine(225 - i, GREEN_MEDIUM);
    }
    drawLine(135 + newHalfAngleTop, GREEN_BRIGHT);
    drawLine(225 - newHalfAngleTop, GREEN_BRIGHT);
  }
  if (newHalfAngleTop < oldHalfAngleTop) {
    for (int8_t i = oldHalfAngleTop; i > newHalfAngleTop; i--) {
      drawLine(135 + i, GREEN_DARK);
      drawLine(225 - i, GREEN_DARK);
    }
    drawLine(135 + newHalfAngleTop, GREEN_BRIGHT);
    drawLine(225 - newHalfAngleTop, GREEN_BRIGHT);
  }
  if (newHalfAngleBottom > oldHalfAngleBottom) {
    for (int8_t i = oldHalfAngleBottom; i < newHalfAngleBottom; i++) {
      drawLine(315 + i, GREEN_MEDIUM);
      drawLine(405 - i, GREEN_MEDIUM);
    }
    drawLine(315 + newHalfAngleBottom, GREEN_BRIGHT);
    drawLine(405 - newHalfAngleBottom, GREEN_BRIGHT);
  }
  if (newHalfAngleBottom < oldHalfAngleBottom) {
    for (int8_t i = oldHalfAngleBottom; i > newHalfAngleBottom; i--) {
      drawLine(315 + i, GREEN_DARK);
      drawLine(405 - i, GREEN_DARK);
    }
    drawLine(315 + newHalfAngleBottom, GREEN_BRIGHT);
    drawLine(405 - newHalfAngleBottom, GREEN_BRIGHT);
  }
  oldHalfAngleBottom = newHalfAngleBottom;
  oldHalfAngleTop = newHalfAngleTop;

}

