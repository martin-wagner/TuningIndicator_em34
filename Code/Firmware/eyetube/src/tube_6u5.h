#ifndef SRC_TUBE_6U5_H_
#define SRC_TUBE_6U5_H_

#include "tube_base.h"

class Tube6u5 : public Eyetube
{
  public:
    Tube6u5(Color color, int32_t pinBacklightPwm, int32_t pinEnable, HardwareSerial &uart, Arduino_GFX *gfx, bool print);

    virtual void run(uint16_t agcRaw, uint16_t brightness) override;

  private:
    uint8_t oldHalfAngle = 0;
    float agcFiltered = 0;

};




#endif /* SRC_TUBE_6U5_H_ */
