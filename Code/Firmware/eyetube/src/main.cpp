/*
 6U5 Digital Replica by retronics.no
 EM34 Digital Replica by Martin Wagner DL2WAG
 */

#include <Arduino.h> 
#include <Arduino_GFX_Library.h> //https://github.com/moononournation/Arduino_GFX | GFX Library for Arduino@^1.4.7
#include <math.h>

#include "tube_base.h"

#define PIN_BACKLIGHT PB0
#define PIN_DC PB11 
#define PIN_CS PA4  
#define PIN_RES PB2
#define PIN_EN PB10
#define PIN_TYPE0 PB5
#define PIN_TYPE1 PB6
#define PIN_TYPE2 PB7
#define PIN_TYPE3 PB8

// PIN_SCL A5
// PIN_SDA A7
#define PIN_AGC PB1
#define BRIGHTNESS 255 // Max 255

Arduino_DataBus *bus = new Arduino_HWSPI(PIN_DC, PIN_CS);
Arduino_GFX *gfx = new Arduino_GC9107(bus, PIN_RES, 0 /* rotation */,
    true /* IPS */, GC9107_TFTWIDTH, GC9107_TFTHEIGHT, 0, 0, 0, 0);
HardwareSerial uart2(PA3, PA2);
Eyetube *tube;

uint8_t fadeInCounter = 0;
uint32_t fadeInLastChange = millis();
uint32_t fade = 0;

void setup(void)
{
  uint8_t type;

  analogWriteResolution(16);
  uart2.begin(115200);
  uart2.println("Digital Eye Tube replacement");
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

  //Display startup
  if (!gfx->begin()) {
    uart2.println("gfx->begin() failed!");
    delay(1000);
    NVIC_SystemReset();
  }
  gfx->fillScreen(BLACK);

  switch (static_cast<Eyetube::Type>(type)) {
    case Eyetube::Type::T_6U5:
      uart2.println("Type 6U5");
      tube = Eyetube::create(Eyetube::Type::T_6U5, Eyetube::Color::C_GREEN,
          PIN_BACKLIGHT, -1, uart2, gfx, false);
      break;
    case Eyetube::Type::T_EM34:
      uart2.println("Type EM34");
//      tube = Eyetube::create(Eyetube::Type::T_EM34, Eyetube::Color::C_GREEN,
//          PIN_BACKLIGHT, -1, uart2, gfx, true);
      tube = Eyetube::create(Eyetube::Type::T_EM34, Eyetube::Color::C_GREEN,
                PIN_BACKLIGHT, PIN_EN, uart2, gfx, false);
      break;
    default:
      uart2.println("Type reading failed");
      uart2.println(
          "Supported types: 6U5 (" + String((int) Eyetube::Type::T_6U5)
              + "), EM64 (" + String((int) Eyetube::Type::T_6U5) + "). Found: "
              + String(type));
      uart2.println("Reboot...");
      delay(1000);
      NVIC_SystemReset();
  }
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

  uint16_t agcRaw = analogRead(PIN_AGC);
  tube->run(agcRaw, fade);
}

