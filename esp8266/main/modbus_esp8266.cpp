#include "modbus_esp8266.h"
#include "driver/rtc.h"
#include "freertos/task.h"

inline uint32_t esp_get_cycle_count() __attribute__((always_inline));
inline uint32_t esp_get_cycle_count() {
  uint32_t ccount;
  __asm__ __volatile__("rsr %0,ccount":"=a"(ccount));
  return ccount;
}

#define ESP8266_REG(addr) *((volatile uint32_t *)(0x60000000+(addr)))
#define GPI    ESP8266_REG(0x318)
#define GPIP(p) ((GPI & (1 << ((p) & 0xF))) != 0)

inline uint32_t F_CPU() {
  auto freq = rtc_clk_cpu_freq_get();
  if (freq == RTC_CPU_FREQ_80M) return 80000000L;
  else if (freq == RTC_CPU_FREQ_160M) return 160000000L;
  else abort();
}

#define WAIT { while (esp_get_cycle_count()-start < wait) { ; }; wait += m_bitTime; }

namespace modbus_esp8266 {

void Driver::init(gpio_num_t _comm_pin, gpio_num_t _rts_pin, gpio_num_t _rtr_pin, 
    uint32_t _speed, int _modbus_timeout)
{
  comm_pin = _comm_pin;
  rts_pin = _rts_pin;
  rtr_pin = _rtr_pin;
  speed = _speed;
  modbus_timeout_in_ms = _modbus_timeout;

  gpio_config_t config;
  config.intr_type = GPIO_INTR_DISABLE;
  config.pin_bit_mask = ((1ULL << comm_pin) | (1ULL << rts_pin) | (1ULL << rtr_pin));
  config.pull_down_en = GPIO_PULLDOWN_DISABLE;
  config.pull_up_en = GPIO_PULLUP_DISABLE;
  config.mode = GPIO_MODE_OUTPUT;
  ESP_ERROR_CHECK(gpio_config(&config));

  gpio_set_level(rts_pin, 0); // disable RS485 driver
  gpio_set_level(rtr_pin, 1); // disable RS485 receiver

  initialized = true;
}

void Driver::send_message(const uint8_t* message, size_t size)
{
  uint32_t m_bitTime = F_CPU() / speed;
  uint32_t startTime = esp_get_cycle_count();
  while (esp_get_cycle_count() - startTime < m_bitTime * 10 * 7 / 2/*3.5 chars silent interval*/) { ; }

  // set commPin to output mode
  gpio_set_level(comm_pin, 1);
  gpio_set_direction(comm_pin, GPIO_MODE_OUTPUT);

  gpio_set_level(rtr_pin, 1); // disable RS485 receiver
  gpio_set_level(rts_pin, 1); // enable RS485 driver

  uint32_t savedLevel = XTOS_DISABLE_ALL_INTERRUPTS;
  for (int i = 0; i < size; i++) {
    //auto orig_prio = uxTaskPriorityGet(NULL);
    //vTaskPrioritySet( NULL, 15);
    //portENTER_CRITICAL();
    uint8_t b = message[i];
    uint32_t wait = m_bitTime;
    uint32_t start = esp_get_cycle_count();
    // Start bit;
    gpio_set_level(comm_pin, 0);
    WAIT;
    for (int j = 0; j < 8; j++) {
      gpio_set_level(comm_pin, (b & 1) ? 1 : 0);
      WAIT;
      b >>= 1;
    }
    // Stop bit
    gpio_set_level(comm_pin, 1);
    WAIT;
  }
  XTOS_RESTORE_INTLEVEL(savedLevel);
  //portEXIT_CRITICAL();
  //vTaskPrioritySet(NULL, orig_prio);

  gpio_set_level(rts_pin, 0); // disable RS485 driver

  startTime = esp_get_cycle_count();
  while (esp_get_cycle_count() - startTime < m_bitTime * 10 * 7 / 2/*3.5 chars silent interval*/) { ; }
}

int Driver::receive_message(uint8_t* modbus_message)
{
  int message_size;
  bool waitingForFirstBit = true;
  uint32_t m_bitTime = F_CPU() / speed;
  uint32_t modbusTimeout = F_CPU() / 1000 * modbus_timeout_in_ms;

  gpio_set_level(rts_pin, 0); // disable RS485 driver
  gpio_set_level(rtr_pin, 0); // enable RS485 receiver
  gpio_set_direction(comm_pin, GPIO_MODE_INPUT);

  uint32_t savedLevel = XTOS_DISABLE_ALL_INTERRUPTS;
  //auto orig_prio = uxTaskPriorityGet(NULL);
  //vTaskPrioritySet( NULL, 15); 
  //portENTER_CRITICAL();

  uint32_t startTime = esp_get_cycle_count();
  for (message_size = 0; message_size < modbus::MAX_MESSAGE_LENGTH; message_size++) {
    startTime = esp_get_cycle_count();
    while (GPIP(comm_pin)) { // wait for a start bit
      if ((esp_get_cycle_count() - startTime) > (waitingForFirstBit? modbusTimeout : (m_bitTime * 10 * 7 / 2)/*3.5 chars silent interval*/)) {
        goto out;
      }
    }
    uint32_t wait = m_bitTime/*start bit*/ + m_bitTime / 2/*middle of next bit*/;
    uint32_t start = esp_get_cycle_count();
    uint8_t rec = 0;
    for (int j = 0; j < 8; j++) {
      WAIT;
      rec >>= 1;
      if (GPIP(comm_pin)) rec |= 0x80;
    }
    // Stop bit
    WAIT;
    waitingForFirstBit = false;
    modbus_message[message_size] = rec;
  }
out:;
  //portEXIT_CRITICAL();
  XTOS_RESTORE_INTLEVEL(savedLevel);
  //vTaskPrioritySet(NULL, orig_prio);

  gpio_set_level(rtr_pin, 1); // disable RS485 receiver
  return message_size;
}

}