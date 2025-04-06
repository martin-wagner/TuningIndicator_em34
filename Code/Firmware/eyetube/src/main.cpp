/*
 6U5 Digital Replica by retronics.no
 EM34 Digital Replica by Martin Wagner DL2WAG
 */

#include <Arduino.h> 
#include <Arduino_GFX_Library.h> //https://github.com/moononournation/Arduino_GFX | GFX Library for Arduino@^1.4.7
#include <math.h>

#define PIN_BACKLIGHT PB0
#define PIN_DC PB11 
#define PIN_CS PA4  
#define PIN_RES PB2
#define PIN_EN PB10
#define PIN_TYPE0 PB5
#define PIN_TYPE1 PB6
#define PIN_TYPE2 PB7
#define PIN_TYPE3 PB8

static constexpr uint8_t TYPE_6U5 = 0;
static constexpr uint8_t TYPE_EM34 = 1;

// PIN_SCL A5
// PIN_SDA A7
#define GREEN_BRIGHT RGB565(0, 255, 80)
#define GREEN_MEDIUM RGB565(0, 180, 60)
#define GREEN_DARK RGB565(10, 80, 35)
#define CENTER_X 63
#define CENTER_Y 75
#define OUTER_RADIUS 52
#define INNER_RADIUS 24
#define PIN_AGC PB1
#define BRIGHTNESS 255 // Max 255

Arduino_DataBus *bus = new Arduino_HWSPI(PIN_DC, PIN_CS);
Arduino_GFX *gfx = new Arduino_GC9107(bus, PIN_RES, 0 /* rotation */,
    true /* IPS */, GC9107_TFTWIDTH, GC9107_TFTHEIGHT, 0, 0, 0, 0);
HardwareSerial uart2(PA3, PA2);
uint8_t oldHalfAngleBottom;
uint8_t oldHalfAngleTop;
float agcFiltered;
uint8_t fadeInCounter = 0;
uint32_t fadeInLastChange = millis();
uint32_t fade = 0;
uint8_t type = 0;

void drawLine(uint16_t angle, uint16_t lineColor);

void setup(void)
{
  analogWriteResolution(16);
  uart2.begin(115200);
  pinMode(PIN_AGC, INPUT);
  pinMode(PIN_BACKLIGHT, OUTPUT);
  pinMode(PIN_EN, INPUT_PULLDOWN);
  pinMode(PIN_TYPE0, INPUT_PULLUP);
  pinMode(PIN_TYPE1, INPUT_PULLUP);
  pinMode(PIN_TYPE2, INPUT_PULLUP);
  pinMode(PIN_TYPE3, INPUT_PULLUP);
  delay(5);
  type = !digitalRead(PIN_TYPE0) | (!digitalRead(PIN_TYPE1) << 1)
      | (!digitalRead(PIN_TYPE2) << 2) | (!digitalRead(PIN_TYPE3) << 3);
  digitalWrite(PIN_BACKLIGHT, LOW);

  switch (type) {
    case TYPE_6U5:
      uart2.println("Digital 6U5 replacement.");
      break;
    case TYPE_EM34:
      uart2.println("Digital EM34 replacement.");
      break;
    default:
      uart2.println("Digital Eye Tube replacement. Type reading failed");
      uart2.println(
          "Supported types: 6U5 (" + String(TYPE_6U5) + "), EM64 ("
              + String(TYPE_EM34) + "). Found: " + String(type));
      uart2.println("Reboot...");
      delay(1000);
      NVIC_SystemReset();
  }

  //Display startup
  if (!gfx->begin()) {
    uart2.println("gfx->begin() failed!");
    delay(1000);
    NVIC_SystemReset();
  }
  gfx->fillScreen(BLACK);
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

  delay(1000);
}

void loop()
{
  //Fade in
  if ((fadeInCounter < BRIGHTNESS)
      && (millis() - fadeInLastChange) > (120 - (fadeInCounter / 4))) {
    fadeInCounter++;
    fade = fadeInCounter * fadeInCounter;
    fadeInLastChange = millis();
  }
  //Enable
  if (digitalRead(PIN_EN)) {
    analogWrite(PIN_BACKLIGHT, fade);
  } else {
    analogWrite(PIN_BACKLIGHT, 0);
  }

  //AGC measurement, filtering and mapping to display angle
  //-0.00321774x^{3}+0.311724x^{2}-9.33456x+90.9922 (0 ... 17V) low sensitivity (bottom)
  //-0.184343x^{3}+4.50276x^{2}-35.8763x+94.963     (0...7V)    high sensivity (top)
  uint16_t agcRaw = analogRead(PIN_AGC);
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

  //todo remove
  //uart2.println("Raw: " + String(agcFiltered) + ", Volt: " + String(x) + ", Angle Bot: " + String(newHalfAngleBottom) + ", Angle Top: " + String(newHalfAngleTop));

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

void drawLine(uint16_t angle, uint16_t lineColor)
{
  double sinVal = sin(angle * 0.0174533);
  double cosVal = cos(angle * 0.0174533);
  gfx->drawLine(CENTER_X + sinVal * INNER_RADIUS,
      CENTER_Y + cosVal * INNER_RADIUS, CENTER_X + sinVal * OUTER_RADIUS,
      CENTER_Y + cosVal * OUTER_RADIUS, lineColor);
  gfx->drawLine(CENTER_X + sinVal * INNER_RADIUS,
      CENTER_Y + 1 + cosVal * INNER_RADIUS, CENTER_X + sinVal * OUTER_RADIUS,
      CENTER_Y + 1 + cosVal * OUTER_RADIUS, lineColor);
}
