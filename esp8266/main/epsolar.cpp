#include "epsolar.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "modbus.h"

static const char* TAG = "epsolar";

namespace epsolar {

const char* Settings::battery_type_str() const
{
    if (battery_type < 0 || battery_type > 3) return NULL;
    static const char* battery_type_str[] = { "User Defined", "Sealed", "GEL", "Flooded/LiFePO4" };
    return battery_type_str[battery_type];
}

double RegisterReadQueryResponse::getDouble(size_t idx) const
{
    auto offset = idx * sizeof(uint16_t);
    if (offset > size - 4) return 0.0;
    return (double)modbus::MKDWORD(modbus::MKWORD(data[offset + 2], data[offset + 3]), modbus::MKWORD(data[offset], data[offset + 1])) / 100.0;
}

uint8_t RegisterReadQueryResponse::getPercentage(size_t idx) const
{
    int16_t value = getInt(idx);
    if (value > 100) value = 100;
    if (value < 0) value = 0;
    return (uint8_t) value;
}

std::pair<uint32_t,uint32_t> RegisterReadQueryResponse::getRTC(size_t idx) const {
    auto offset = idx * sizeof(uint16_t);
    if (offset > size - 6) return std::make_pair(0, 0);
    uint16_t r9013 = getWord(idx);
    uint16_t r9014 = getWord(idx + 1);
    uint16_t r9015 = getWord(idx + 2);
    uint16_t year = 2000 + (r9015 >> 8);
    uint8_t month = r9015 & 0x00ff;
    uint8_t day = r9014 >> 8;
    uint8_t hour = r9014 & 0x00ff;
    uint8_t minute = r9013 >> 8;
    uint8_t second = r9013 & 0x00ff;
    return std::make_pair(year * 10000L + month * 100 + day, hour * 10000L + minute * 100 + second);
}

bool read_settings(Settings& settings)
{
    RegisterReadQueryResponse reg;
    if (!modbus::read_query(1, 3/*Read Input Register*/, 0x9000/*battery type, battery capacity*/, 14, reg)) return false;

    settings.battery_type = reg.getInt(0);
    settings.battery_capacity = reg.getInt(1);
    settings.temperature_compensation_coefficient = reg.getFloat(2);

    if (settings.battery_type == 0/*User defined*/) {
        settings.boost_voltage = reg.getFloat(7)/*0x9007*/ /* * multiplier*/;
        settings.float_voltage = reg.getFloat(8)/*0x9008*/ /* * multiplier*/;
        settings.low_voltage_disconnect = reg.getFloat(13)/*0x900d*/ /* * multiplier*/;
    }

    if (!modbus::read_query(1, 4/*Read Input Register*/, 0x311d/*Battery real rated voltage*/, 1, reg)) return false;
    settings.battery_rated_voltage = (uint8_t)reg.getFloat(0);
    return true;
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

    if (!modbus::read_query(1, 4, 0x3200, 2, reg)) return false; // battery_voltage_status, Charging equipment status
    //else
    values.battery_voltage_status = reg.getWord(0) & 0x000f;
    values.cs = (reg.getWord(1) >> 2) & 0x0003;

    if (!modbus::read_query(1, 4, 0x3304, 9, reg)) return false;
    //else
    values.lkwh = reg.getDouble(0); // 0x3304
    values.kwh = reg.getDouble(8); // 0x330c

    if (!modbus::read_query(1, 3/*Read Holding Register*/, 0x9013/*Real Time Clock*/, 3, reg)) return false;
    auto rtc = reg.getRTC(0);
    values.d = rtc.first;
    values.t = rtc.second;

    modbus::StatusReadQueryResponse reg2;
    if (!modbus::read_query(1, 1/*Read Coil Status*/, 0x0002, 1, reg2)) return false; // Manual control the load
    // else
    values.pw = reg2.getBool(0)? 1 : 0;

    return true;
}

bool set_temperature_compensation_coefficient(uint16_t mv)
{
    return modbus::write_query(1, 6/*Preset single register*/, 
        0x9002/*Temperature compensation coefficient*/, mv);
}

bool set_battery_type(int battery_type)
{
    if (battery_type < 0 || battery_type > 6) {
        ESP_LOGE(TAG, "Invalid battery type: %d", battery_type);
        return false;
    }
    //else
    if ((battery_type > 0 && battery_type <= 3) || battery_type == 6/*LiFePO4*/) {
        if (!modbus::write_query(1, 6/*Preset single register*/, 0x9000/*Battery type*/, 
            (uint16_t)(battery_type == 6/*LiFePO4 pretends flooded battery*/? 3 : battery_type))) {
            ESP_LOGE(TAG, "Failed to set battery type(device not connected?)");
            return false;
        }
        vTaskDelay(100 / portTICK_RATE_MS);
        uint16_t temperature_compensation_coefficient = (battery_type == 6? 0 : 300);
        if (!set_temperature_compensation_coefficient(temperature_compensation_coefficient)) {
            ESP_LOGE(TAG, "Failed to set temperature compensation coefficient(device not connected?)");
            return false;
        }
    } else if (battery_type == 4/*3S / 6S*/ || battery_type == 5/*7S*/) {
        RegisterReadQueryResponse reg;
        if (!modbus::read_query(1, 4/*Read Input Register*/, 0x311d, 1, reg)) {
            ESP_LOGE(TAG, "Failed to read battery real rated voltage(0x311d)");
            return false;
        }
        //else
        auto battery_rated_voltage = reg.getFloat(0);

        // 9002-900e
        uint16_t data[13] = {
          0, // Temperature compensation coefficient
          1341, // High Volt.disconnect
          1250, // Charging limit voltage
          1250, // Over voltage reconnect
          1240, // Equalization voltage
          1240, // Boost voltage
          1230, // Float voltage (originally 1180)
          1120, // Boost reconnect voltage
          1040, // Low voltage reconnect
          1020, // Under voltage recover
          1000, // Under voltage warning
          920, // Low voltage disconnect
          900, // Discharging limit voltage
        };
        const char* num_series = "3S";
        if (battery_type == 4 && battery_rated_voltage > 17.0) { // 6S for 24V system
          for (int i = 0; i < sizeof(data) / sizeof(data[0]); i++) {
            data[i] *= 2;
          }
          num_series = "6S";
        } else if (battery_type == 5) {
          for (int i = 0; i < sizeof(data) / sizeof(data[0]); i++) {
            data[i] = data[i] * 7 / 3;
          }
          num_series = "7S";
        }
        if (!modbus::write_query(1, 6, 0x9000/*battery type*/, 0)) { // 0x9000=Battery type, 0=user
            ESP_LOGE(TAG, "Failed to set battery type(device not connected?)");
            return false;
        }
        vTaskDelay(100 / portTICK_RATE_MS);
        if (!modbus::write_query(1, 0x9002, data, sizeof(data) / sizeof(data[0]))) {
            ESP_LOGE(TAG, "User setting error for %s", num_series);
            return false;
        }
    }

    return true;
}

void set_battery_capacity(int battery_capacity)
{
    if (modbus::write_query(1, 6/*Preset single register*/, 0x9001/*Battery capacity*/, 
        (uint16_t)battery_capacity)) {
        printf("Battery capacity saved: %dAh\n", battery_capacity);
    } else {
        ESP_LOGE(TAG, "Saving battery capacity failed(device not connected?)");
    }
}

void set_load_switch(bool on)
{
    if (modbus::write_query(1, 5/*Write Single Coil*/, 0x0002/*manual load control*/, on? (uint16_t)0xff00:(uint16_t)0x0000)) {
        printf("Power %s\n", on? "ON" : "OFF");
    } else {
        ESP_LOGE(TAG, "Turning %s load failed(device not connected?)", on? "ON" : "OFF");
    }
}

void set_datetime(int32_t date, int32_t time)
{
    uint16_t year = date / 10000 - 2000;
    uint16_t month = date % 10000 / 100;
    uint16_t day = date % 100;
    uint16_t hour = time / 10000;
    uint16_t minute = time % 10000 / 100;
    uint16_t second = time % 100;

    if (year > 99 || month < 1 || month > 12 || day < 1 || day > 31 || hour > 23 || minute > 59 || second > 59) {
        ESP_LOGE(TAG, "Invalid date and time(%d %d) specified.", date, time);
    }
    //else
    uint16_t data[3];
    data[0/*0x9013*/] = minute << 8 | second;
    data[1/*0x9014*/] = day << 8 | hour;
    data[2/*0x9015*/] = year << 8 | month;

    if (modbus::write_query(1, 0x9013, data, 3)) {
        printf("Date saved: 20%02u-%02u-%02u %02u:%02u:%02u\n", year, month, day, hour, minute, second);
    } else {
        ESP_LOGE(TAG, "RTC was not saved(invalid date or communication error).");
    }
}

}