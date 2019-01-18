#include <Arduino.h>

#if defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>

#include "globals.h"

static long last_display_time = 0;
static TaskHandle_t backgroundTask;

static SemaphoreHandle_t mutex = NULL;
static uint32_t count = 0;

class MutexHolder {
  const SemaphoreHandle_t& mutex;
public:
  MutexHolder(SemaphoreHandle_t _mutex) : mutex(_mutex) { xSemaphoreTake(mutex, portMAX_DELAY); }
  ~MutexHolder() { xSemaphoreGive(mutex); }
};

static void background(void *pvParams)
{
  modbus.enter_slave();

  uint8_t modbus_message[MAX_MODBUS_MESSAGE_LENGTH];

  while (true) {
    int message_size = modbus.receive_modbus_message(modbus_message);
    if (message_size < 1) continue;
    if (modbus_message[0] != config.slave_id) {
      Serial.print("modbus: Not for me (");
      Serial.print((int)message_size);
      Serial.println(" bytes)");
      {
        MutexHolder x(mutex);
        count++;
      }
      continue;
    }
    if (message_size < 4) {
      Serial.println("modbus: Message too short");
      continue;
    }
    Serial.println("Hello, World!");
  }
}

void setup_slave()
{
  WiFi.mode(WIFI_OFF);
  btStop();

  mutex = xSemaphoreCreateMutex();
  xTaskCreatePinnedToCore(background, "background", 4096, NULL, 5, &backgroundTask, 0);
}

void loop_slave()
{
  long current_time = millis();
  if (current_time - last_display_time > 1000) {
    display.clearDisplay();
    display.setCursor(0,0);
    display.println("Messages detected:");
    {
      MutexHolder x(mutex);
      display.println(count);
    }
    display.display();
    last_display_time = current_time;
  }
  delay(100);
}

#else
void setup_slave()
{
  Serial.println("Slave operation mode is not supported in this architecture.");
}
void loop_slave()
{
}
#endif // ARDUINO_ARCH_ESP32
