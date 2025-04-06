#ifndef SRC_TUBE_BASE_H_
#define SRC_TUBE_BASE_H_

#include <Arduino.h>
#include <Arduino_GFX_Library.h> //https://github.com/moononournation/Arduino_GFX | GFX Library for Arduino@^1.4.7

class Eyetube {
  public:
    enum class Type {
      T_6U5 = 0,
      T_EM34 = 1,
    };
    enum class Color {
      C_GREEN,
      C_YELLOW,
      C_BLUE
    };

    static constexpr uint16_t GREEN_BRIGHT = RGB565(0, 255, 80);
    static constexpr uint16_t GREEN_MEDIUM = RGB565(0, 180, 60);
    static constexpr uint16_t GREEN_DARK = RGB565(10, 80, 35);

  public:

    /** factory */
    static Eyetube *create(Type type, Color color, int32_t pinBacklightPwm, int32_t pinEnable, HardwareSerial &uart, Arduino_GFX *gfx, bool print = false);

    /**
     * update display
     *
     * @param agcRaw unfiltered agc raw value 0 ... 1023
     * @param brightness brightness 0 ... 65535
     */
    virtual void run(uint16_t agcRaw, uint16_t brightness) = 0;

  protected:
    static constexpr uint8_t CENTER_X = 63;
    static constexpr uint8_t CENTER_Y = 75;
    static constexpr uint8_t OUTER_RADIUS = 52;
    static constexpr uint8_t INNER_RADIUS = 24;

    Eyetube(Color color, int32_t pinBacklightPwm, int32_t pinEnable, HardwareSerial &uart, Arduino_GFX *gfx, bool print) :
      pinBacklightPwm(pinBacklightPwm), pinEnable(pinEnable), uart(uart), gfx(gfx), print(print)
    {
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

  protected:
    int32_t pinBacklightPwm;
    int32_t pinEnable;
    HardwareSerial &uart;
    Arduino_GFX *gfx;
    bool print;
};

#endif /* SRC_TUBE_BASE_H_ */
