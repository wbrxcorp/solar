#ifndef __EPSOLAR_H__
#define __EPSOLAR_H__

#include "modbus.h"

namespace epsolar {

struct Settings {
  int battery_type;
  int battery_capacity;
  const char* battery_type_str() const;

  float temperature_compensation_coefficient;
  float boost_voltage;
  float float_voltage;
  float low_voltage_disconnect;

  uint8_t battery_rated_voltage;
};

struct Values {
  float piv, pia, bv, poa, temp;
  double piw, load, lkwh, kwh;
  uint8_t soc;
  uint16_t cs;
  bool pw;

  uint32_t d/*date*/, t/*time*/;
  uint8_t battery_voltage_status;
};

class RegisterReadQueryResponse : public modbus::RegisterReadQueryResponse {
public:
  int16_t getInt(size_t idx) const {return (int16_t)getWord(idx); }
  float getFloat(size_t idx) const { return (float)getInt(idx) / 100.0f; }
  double getDouble(size_t idx) const;
  uint8_t getPercentage(size_t idx) const;
  std::pair<uint32_t,uint32_t> getRTC(size_t idx) const;
};

bool read_settings(Settings& settings);
bool read_values(Values& values);
bool set_battery_type(int battery_type);
void set_battery_capacity(int battery_capacity);
void set_load_switch(bool on);
void set_datetime(int32_t date, int32_t time);

} // namespace epsolar

#endif