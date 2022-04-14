#include "epsolar.h"
#include "modbus.h"

namespace epsolar {

double RegisterReadQueryResponse::getDouble(size_t idx) const
{
    auto offset = idx * sizeof(uint16_t);
    if (offset > size - 4) return 0.0;
    return (double)modbus::MKDWORD(modbus::MKWORD(data[offset + 2], data[offset + 3]), modbus::MKWORD(data[offset], data[offset + 1])) / 100.0;
}

bool read_values(Values& values)
{
    RegisterReadQueryResponse reg;
    // TODO: apply https://github.com/wbrxcorp/solar/commit/a664b0577e7c9bbcac6448741c43e58ce6750e43
    if (!modbus::read_query(1, 4/*Read Input Register*/, 0x3100, 6, reg)) return false;
    // else
    values.piv = reg.getFloat(0);
    values.pia = reg.getFloat(1);
    values.piw = reg.getDouble(2);
    values.bv = reg.getFloat(4);
    values.poa = reg.getFloat(8);

    if (!modbus::read_query(1, 4, 0x310e, 3, reg)) return false;
    //else
    values.load = reg.getDouble(0);
    values.temp = reg.getFloat(2);

    if (!modbus::read_query(1, 4, 0x311a, 1, reg)) return false;
    values.soc = reg.getPercentage(0);

    if (!modbus::read_query(1, 4, 0x3201, 1, reg)) return false; // Charging equipment status
    //else
    values.cs = (reg.getWord(0) >> 2) & 0x0003;

    if (!modbus::read_query(1, 4, 0x3304, 1, reg)) return false;
    //else
    values.lkwh = reg.getDouble(0);

    if (!modbus::read_query(1, 4, 0x330c, 1, reg)) return false;
    values.kwh = reg.getDouble(0);

    modbus::StatusReadQueryResponse reg2;
    if (!modbus::read_query(1, 1/*Read Coil Status*/, 0x0002, 1, reg2)) return false; // Manual control the load
    // else
    values.pw = reg2.getBool(0)? 1 : 0;

    return true;
}

}