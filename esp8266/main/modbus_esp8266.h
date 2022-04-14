#ifndef __MODBUS_ESP8266_H__
#define __MODBUS_ESP8266_H__

#include "modbus.h"
#include "driver/gpio.h"

namespace modbus_esp8266 {

class Driver : public modbus::Driver {
    bool initialized;
    gpio_num_t comm_pin, rts_pin, rtr_pin;
    uint16_t modbus_timeout_in_ms;
    uint32_t speed;
public:
    Driver() : initialized(false) {;}
    void init(gpio_num_t _comm_pin = GPIO_NUM_0, 
        gpio_num_t _rts_pin = GPIO_NUM_15, gpio_num_t _rtr_pin = GPIO_NUM_2, 
        uint32_t _speed = 115200, int _modbus_timeout = 100);
    virtual void send_message(const uint8_t* message, size_t size);
    virtual int receive_message(uint8_t* modbus_message);
};

}

#endif // __MODBUS_ESP8266_H__