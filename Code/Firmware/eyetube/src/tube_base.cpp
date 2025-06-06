#include "tube_base.h"
#include "tube_em34.h"

Eyetube* Eyetube::create(Type type, Color color, int32_t pinBacklightPwm,
    int32_t pinEnable, HardwareSerial &uart, Arduino_GFX *gfx, bool print)
{
  switch (type) {
    case Type::T_EM34:
      return new TubeEm34(color, pinBacklightPwm, pinEnable, uart, gfx, print);
    default:
      return nullptr;
  }
}
