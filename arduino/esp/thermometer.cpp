#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#include <SPIFFS.h>
#include <esp32-hal-ledc.h>
static TaskHandle_t ledTask;
static SemaphoreHandle_t mutex = NULL;
static uint16_t eCO2_shared = 0;
#endif
#include <FS.h>

#include "globals.h"

#define BME280_I2C_ADDRESS 0x76 // GYBMEP
#define CCS811_I2C_ADDRESS 0x5A

static unsigned long last_update = 0;
static bool ccs811_present = false;

static const uint16_t interval = 1000;
static const uint8_t red_pin = 33;
static const uint8_t yellow_pin = 25;
static const uint8_t green_pin = 26;


void early_setup_thermometer()
{
  WiFi.mode(WIFI_OFF);
  tft.setRotation(config.tft_rotation);
#if defined(ARDUINO_ARCH_ESP8266)
  tft.begin((int8_t)0, (int8_t)15);
#elif defined(ARDUINO_ARCH_ESP32)
  tft.begin((int8_t)5, (int8_t)0);
#endif
}

// for 4M1M esp8266
// mkspiffs -c . -b 8192 -p 256 -s 0xfb000 /tmp/esp8266.spiffs
// esptool.py --baud 460800 --port /dev/ttyUSB0 write_flash 0x300000 /tmp/esp8266.spiffs
//
// mkspiffs -c . -b 4096 -p 256 -s 0x16f000 /tmp/esp32.spiffs
// esptool.py --chip esp32 --baud 460800 --port /dev/ttyUSB0 write_flash 0x291000 /tmp/esp32.spiffs
void load_background_image()
{
  Serial.println("Loading background image from SPIFFS...");
  SPIFFS.begin();
  File f = SPIFFS.open("/background.bmp", "r");
  bool loaded = false;
  if (f) {
    loaded = tft.showBitmapFile(f);
    f.close();
  }

  if (loaded) {
    Serial.println("background.bmp loaded");
  } else {
    Serial.println("background.bmp not loaded");
    tft.fillScreen(TFT_WHITE);
  }
  SPIFFS.end();
}

static uint8_t get_duty(double ratio){
  return (uint8_t)round(exp(log(255.0) - (1 - ratio) * log(255.0/*C*/))) - 1;
}

#ifdef ARDUINO_ARCH_ESP32
static void led(void *pvParams)
{
  int32_t green, yellow, red;
  while (true) {
    xSemaphoreTake(mutex, portMAX_DELAY);
    uint16_t eCO2 = eCO2_shared;
    xSemaphoreGive(mutex);

    if (eCO2 == 0) {
      green = yellow = red = 255;
    } else {
      unsigned long t = millis();
      green = (300 - (eCO2 - 700)) * 255 / 300;
      if (green < 0) green = 0;
      if (green > 255) { // blink
        if (t / 1000 % 2) { // on
          green = 255;
        } else { // off
          green = (1000 - t % 1000) * 255 / 1000;
        }
      }

      yellow = eCO2 >= 1000? ((1500 - eCO2) * 255 / 500) : ((eCO2 - 700) * 255 / 300);
      if (yellow < 0) yellow = 0;
      if (yellow > 255) yellow = 255;

      red = (eCO2 - 1000) * 255 / 500;
      if (red < 0) red = 0;
      if (red > 255) { // blink
        int interval;
        if (eCO2 >= 3000) interval = 250;
        else if (eCO2 >= 2000) interval = 500;
        else interval = 1000;

        if (t / interval % 2) {
          red = 255;
        } else {
          red = (interval - t % interval) * 255 / interval;
        }
      }
    }

    ledcWrite(0, get_duty(red / 255.0));
    ledcWrite(1, get_duty(yellow / 255.0));
    ledcWrite(2, get_duty(green / 255.0));
    delay(10);
  }
}
#endif

void setup_thermometer()
{
  load_background_image();

  if (!bme.begin(BME280_I2C_ADDRESS)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.println("SENSOR NOT FOUND");
    tft.println("REBOOT AFTER 5SEC");
#if defined(ARDUINO_ARCH_ESP8266)
    ESP.deepSleep(5 * 1000L * 1000L , WAKE_RF_DEFAULT);
#elif defined(ARDUINO_ARCH_ESP32)
    esp_sleep_enable_timer_wakeup(5 * 1000L * 1000L);
    esp_deep_sleep_start();
#endif
    delay(1000);
  }

  if (ccs.begin(CCS811_I2C_ADDRESS)) {
    ccs811_present = true;
    while(!ccs.available());
    float temp = ccs.calculateTemperature();
    ccs.setTempOffset(temp - 25.0);
#ifdef ARDUINO_ARCH_ESP32
    ledcSetup(0, 5000, 8);
    ledcAttachPin(red_pin, 0);
    ledcSetup(1, 5000, 8);
    ledcAttachPin(yellow_pin, 1);
    ledcSetup(2, 5000, 8);
    ledcAttachPin(green_pin, 2);
    mutex = xSemaphoreCreateMutex();
    xTaskCreatePinnedToCore(led, "led", 4096, NULL, 5, &ledTask, 0);
#endif

  } else {
    Serial.println("Could not find a CCS811 sensor.");
  }
}

void thermometer_print_values(float temperature, float humidity, uint16_t pressure)
{
  char buf[16];
  sprintf(buf, "%2.1f", temperature);
  char* decimal_part = strchr(buf, '.');

  if (decimal_part) {
    // decimal part first
    tft.setTextColor(TFT_RED, TFT_WHITE);
    tft.setTextSize(3);
    tft.setCursor(108, 20);
    tft.print(decimal_part);

    *decimal_part = '\0';
  }

  tft.setTextSize(8);
  tft.setCursor(10, 16);
  tft.print(buf);

  // humidity
  sprintf(buf, "%2.1f", humidity);
  tft.setTextSize(3);
  tft.setCursor(13, 122);
  tft.setTextColor(TFT_BLUE, TFT_WHITE);
  tft.print(buf);

  // pressure
  sprintf(buf, "%4d", pressure);
  tft.setTextSize(3);
  tft.setCursor(10, 155);
  tft.setTextColor(TFT_GREEN, TFT_WHITE);
  tft.print(buf);
}

void gasmeter_print_values(uint16_t co2, float tvoc, float temperature)
{
  char buf[16];

  // eCO2
  sprintf(buf, "%4d", (int)co2);
  tft.setCursor(10, 188);
  tft.setTextColor(TFT_ORANGE, TFT_WHITE);
  tft.print(buf);

  // TVOC
  Serial.print(" ppm, TVOC: ");
  Serial.print(tvoc);

  // CCS811's temperature
  Serial.print(" ppb   Temp:");
  Serial.println(temperature);
}

void loop_thermometer()
{
  unsigned long current_time = millis();
  if (current_time < last_update + interval) return;
  // else
  float temperature = bme.readTemperature();
  float humidity =  bme.readHumidity();
  thermometer_print_values(temperature, humidity, (uint16_t)(bme.readPressure() / 100.0F));

  if (ccs811_present) {
    ccs.setEnvironmentalData((uint8_t)humidity, temperature);
    if (ccs.available()) {
      float temp = ccs.calculateTemperature();
      if(!ccs.readData()){
        uint16_t eCO2 = ccs.geteCO2();
#ifdef ARDUINO_ARCH_ESP32
        xSemaphoreTake(mutex, portMAX_DELAY);
        eCO2_shared = eCO2;
        xSemaphoreGive(mutex);
#endif
        gasmeter_print_values(eCO2, ccs.getTVOC(), temp);
      }
    }
  }
  last_update = current_time;
}
