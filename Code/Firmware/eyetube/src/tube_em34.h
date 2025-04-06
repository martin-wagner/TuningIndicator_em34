#ifndef SRC_TUBE_EM34_H_
#define SRC_TUBE_EM34_H_

#include "tube_base.h"

class TubeEm34 : public Eyetube
{
  public:
    TubeEm34(Color color, int32_t pinBacklightPwm, int32_t pinEnable, HardwareSerial &uart, Arduino_GFX *gfx, bool print);

    virtual void run(uint16_t agcRaw, uint16_t brightness) override;

  private:
    uint8_t oldHalfAngleBottom = 0;
    uint8_t oldHalfAngleTop = 0;
    float agcFiltered = 0;

};




#endif /* SRC_TUBE_EM34_H_ */
