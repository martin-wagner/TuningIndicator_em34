#include "tube_base.h"
#include "tube_em34.h"
#include "tube_6u5.h"

Eyetube* Eyetube::create(Type type, Color color, int32_t pinBacklightPwm,
    int32_t pinEnable, HardwareSerial &uart, Arduino_GFX *gfx, bool print)
{
  switch (type) {
    case Type::T_EM34_GREEN:
    case Type::T_EM34_YELLOW:
    case Type::T_EM34_BLUE:
      return new TubeEm34(color, pinBacklightPwm, pinEnable, uart, gfx, print);
    case Type::T_6U5:
      return new Tube6u5(color, pinBacklightPwm, pinEnable, uart, gfx, print);
    default:
      return nullptr;
  }
}
