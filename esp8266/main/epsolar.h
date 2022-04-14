#ifndef __EPSOLAR_H__
#define __EPSOLAR_H__

#include "modbus.h"

namespace epsolar {

struct Values {
  float piv, pia, bv, poa, temp;
  double piw, load, lkwh, kwh;
  uint8_t soc;
  uint16_t cs;
  bool pw;
};

class RegisterReadQueryResponse : public modbus::RegisterReadQueryResponse {
public:
  int16_t getInt(size_t idx) const {
    return (int16_t)getWord(idx);
  }

  float getFloat(size_t idx) const {
    return (float)getInt(idx) / 100.0f;
  }

  double getDouble(size_t idx) const;

  uint8_t getPercentage(size_t idx) const {
    int16_t value = getInt(idx);
    if (value > 100) value = 100;
    if (value < 0) value = 0;
    return (uint8_t) value;
  }
};

bool read_values(Values& values);

} // namespace epsolar

#endif