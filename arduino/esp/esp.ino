// ESP8266(2M1M or 4M1M)
// arduino --upload --board esp8266com:esp8266:generic:xtal=80,FlashMode=qio,FlashFreq=40,ResetMethod=nodemcu,eesz=2M1M,baud=460800 --port /dev/ttyUSB0 .
// ESP32
// arduino --upload --board espressif:esp32:esp32:FlashFreq=40,UploadSpeed=921600 --port /dev/ttyUSB0 .
#include <EEPROM.h>
#ifdef ARDUINO_ARCH_ESP8266
#include <ESP8266WiFi.h>
#include <user_interface.h>
#elif defined ARDUINO_ARCH_ESP32
#include <WiFi.h>
#include <esp_wifi.h>
#include <rom/rtc.h>

#define REASON_DEFAULT_RST POWERON_RESET
#define REASON_DEEP_SLEEP_AWAKE DEEPSLEEP_RESET
#endif

#define RS485_COMM_SOCKET 0

#if defined(ARDUINO_ARCH_ESP8266)
  #define RS485_RE_SOCKET 2   // default pull-up(Receiver disable) in ESP8266
  #define RS485_DE_SOCKET 15  // default pull-down(Driver disable) in ESP8266

  #define PW1_SW_SOCKET 14    // ESP8266 IO14 D1 mini D5
  #define PW1_LED_SOCKET 12   // ESP8266 IO12 D1 mini D6

  #define GPIO_SOCKET 13
#elif defined(ARDUINO_ARCH_ESP32)
  #define RS485_TX_SOCKET 17
  #define RS485_RX_SOCKET 16
  #define RS485_RE_SOCKET 5   // default pull-up(Receiver disable) in ESP32
  #define RS485_DE_SOCKET 2   // default pull-down(Driver disable) in ESP32

  #define PW1_SW_SOCKET 18
  #define PW1_LED_SOCKET 19

  #define GPIO_SOCKET 23
#endif

#define DISPLAY_I2C_ADDRESS 0x3c

#define CHECK_INTERVAL 1000
#define REPORT_INTERVAL 5000
#define MESSAGE_TIMEOUT 10000
#define COMMAND_LINE_ONLY_MODE_WAIT_SECONDS 3
#define LOW_BATTERY_SLEEP_SECONDS (60 * 5)
#define MAX_POSTPONE_COUNT 12

const char* DEFAULT_NODENAME = "kennel01";
const char* DEFAULT_SERVERNAME = "_solar._tcp";
const uint16_t DEFAULT_PORT = 29574; // default server port number
const uint8_t DEFAULT_SLAVE_ID = 2;
const uint8_t DEFAULT_TFT_ROTATION = 2;

#include "network.h"
#include "command_line.h"
#include "server.h"
#include "nisetracer.h"
#include "edogawa_master.h"
#include "thermometer.h"
#include "ammeter.h"
#include "slave.h"
#include "watchdog.h"
#include "crc.h"

#include "globals.h"
#include "epsolar.h"

static EPSolar epsolar(modbus);

String cmdline_buffer;

static unsigned long last_message_sent = 0L;
static unsigned long last_message_received = 0L;
static unsigned long last_checked = 0L;
static unsigned long last_reported = 0L;
static float last_bv = 0.0f;
static char session_id[48] = "";
static int reset_reason = REASON_DEFAULT_RST;
static uint8_t battery_voltage_status = 0x00; // 0x3200-D3-D0: 0=Normal, 1=Overvolt, 2=Undelvolt, 3=Low volt Disconnect, 4=Fault
static bool gpio_output_mode = false;
static bool gpio_output_value = false;

static bool ina219_started = false;

typedef struct strEPSolarValues {
  float piv,pia,bv,poa;
  double piw;
  double load;
  float temp;
  int cs;
  double lkwh;
  double kwh;
  int pw;
  uint8_t soc;
} EPSolarValues;

struct {
  // --- 0
  uint8_t valid;
  uint8_t channel;
  uint8_t bssid[6];
  // --- 8
  uint8_t battery_rated_voltage = 0; // 12 or 24(V)
  uint8_t temperature_compensation_coefficient = 0; // 0-9(mV)
  // --- 10
  char server_addr[46]; // maximum IPv6 string length is 45 characters
  uint16_t server_port;
  // --- 58
  uint16_t postpone_count;
  // --- 60
  char pad[2];
  // --- 62
  uint16_t crc;
  // size have to be multiple of 4
} rtcData;

static void set_battery_voltage_status(uint8_t value)
{
  battery_voltage_status = value;
}

static bool is_battery_low()
{
  return battery_voltage_status == 0x03/*low voltage disconnect*/ || battery_voltage_status == 0x04/*fault*/;
}

static const char* get_battery_voltage_status_string()
{
  static const char* status_string[] = {
    "Normal","Overvolt","Undervolt","Low Volt Disconnect","Fault"
  };
  return (battery_voltage_status < 5? status_string[battery_voltage_status] : "Unknown");
}

static void process_message(const char* message)
{
  Serial.print("Received: ");
  Serial.println(message);

  if (strlen(message) < 3 || strncmp(message, "OK\t", 3) != 0) return;
  // else
  const char* pt = message + 3;
  int32_t date = -1, time = -1;
  uint16_t sleep_sec = 0;

  while (*pt) {
    const char* ptcolon = strchr(pt, ':');
    if (ptcolon == NULL) break; // end parsing string if there's no colon anymore
    char key[ptcolon - pt + 1];
    strncpy(key, pt, ptcolon - pt);
    key[ptcolon - pt] = '\0';
    pt = ptcolon + 1;
    const char* ptdelim = strchr(pt, '\t');
    if (ptdelim == NULL) ptdelim = strchr(pt, '\0');
    // ptdelim can't be NULL here
    char value[ptdelim - pt + 1];
    strncpy(value, pt, ptdelim - pt);
    value[ptdelim - pt] = '\0';
    pt = (*ptdelim != '\0') ? ptdelim + 1 : ptdelim;

    if (strcmp(key, "session") == 0) {
      strncpy(session_id, value, sizeof(session_id));
      session_id[sizeof(session_id) - 1] = '\0';
    } else if (strcmp(key, "d") == 0 && strlen(value) == 8 && isdigit(value[0])) {
      date = atol(value);
    } else if (strcmp(key, "t") == 0 && strlen(value) > 0 && isdigit(value[0])) {
      time = atol(value);
    } else if (strcmp(key, "bt") == 0 && reset_reason != REASON_DEEP_SLEEP_AWAKE) {
      int battery_type = atoi(value);
      if ((battery_type > 0 && battery_type <= 3) || battery_type == 6/*LiFePO4*/) {
        epsolar.put_register(0x9000/*Battery type*/, (uint16_t)(battery_type == 6/*LiFePO4 pretends flooded battery*/? 3 : battery_type));
        Serial.print("Battery type saved: ");
        Serial.println(battery_type);
        delay(100);
        if (battery_type == 6/*LiFePO4 doesn't need this feature*/) {
          epsolar.put_register(0x9002/*Temperature compensation coefficient*/, 0);
          Serial.println("Temperature compensation coefficient disabled");
        } else {
          epsolar.put_register(0x9002/*Temperature compensation coefficient*/, 300);
          Serial.println("Temperature compensation coefficient set to 3mV/C/2V");
        }
      } else if (battery_type == 4/*3S / 6S*/ || battery_type == 5/*7S*/) {
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
        if (battery_type == 4 && rtcData.battery_rated_voltage > 17.0) { // 6S for 24V system
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
        if (epsolar.put_register(0x9000/*Battery type*/, 0/*user*/)) {
          delay(100);
          if (epsolar.put_registers(0x9002, data, sizeof(data) / sizeof(data[0]))) {
            Serial.print("Battery type saved: ");
            Serial.println(num_series);
          } else {
            Serial.print("User setting error for ");
            Serial.println(num_series);
          }
        } else {
          Serial.println("Unable to set to user-defined mode");
        }
      } else {
        Serial.print("Invalid battery type: ");
        Serial.println(battery_type);
      }
    } else if (strcmp(key, "bc") == 0 && reset_reason != REASON_DEEP_SLEEP_AWAKE) {
      int battery_capacity= atoi(value);
      epsolar.put_register(0x9001/*Battery capacity*/, (uint16_t)battery_capacity);
      Serial.print("Battery capacity saved: ");
      Serial.print(battery_capacity);
      Serial.println("Ah");
    } else if (strcmp(key, "pw") == 0 && isdigit(value[0])) { // main power
      int pw = atoi(value);
      if (pw == 0) {
        Serial.println("Power OFF");
        if (edogawaUnit1 && edogawaUnit1.is_power_on()) edogawaUnit1.power_off(); // atx power off
        if (edogawaUnit2 && edogawaUnit2.is_power_on()) edogawaUnit2.power_off(); // atx power off
        epsolar.load_on(false);
      } else if (pw == 1) {
        Serial.println("Power ON");
        epsolar.load_on(true);
      }
    } else if (key[0] == 'p' && key[1] == 'w' && (key[2] == '1' || key[2] == '2') && (isdigit(value[0]) || value[0] == '-')) { // atx power
      EdogawaUnit& edogawaUnit = key[2] == '1'? edogawaUnit1 : edogawaUnit2;
      if (edogawaUnit) {
        int pwX = atoi(value);
        bool pwX_on = edogawaUnit.is_power_on();
        if (pwX_on && pwX == 0) { // power off
          Serial.print("Power");
          Serial.print(key[2]);
          Serial.println(" OFF");
          edogawaUnit.power_off();
        } else if (!pwX_on && pwX == 1) { // power on
          Serial.print("Power");
          Serial.print(key[2]);
          Serial.println(" ON");
          epsolar.load_on(true); // main power on first
          edogawaUnit.power_on();
        }
      }
    } else if (strcmp(key, "sleep") == 0 && isdigit(value[0])) {
        sleep_sec = (int16_t)atoi(value);
    } else if (strcmp(key, "gpio") == 0 && (value[0] == '0' || value[0] == '1') && value[1] == '\0') {
      if (!gpio_output_mode) {
        gpio_output_mode = true;
        pinMode(GPIO_SOCKET, OUTPUT);
      }
      gpio_output_value = value[0] == '0' ? 0 : 1;
      digitalWrite(GPIO_SOCKET, gpio_output_value? HIGH : LOW);
    }
  }

  if (reset_reason != REASON_DEEP_SLEEP_AWAKE && date > 20170101L && time >= 0) {
    uint16_t year = date / 10000 - 2000;
    uint16_t month = date % 10000 / 100;
    uint16_t day = date % 100;
    uint16_t hour = time / 10000;
    uint16_t minute = time % 10000 / 100;
    uint16_t second = time % 100;

    if (epsolar.set_rtc(year, month, day, hour, minute, second)) {
      char buf[32];
      sprintf(buf, "Date saved: 20%02u-%02u-%02u %02u:%02u:%02u", year, month, day, hour, minute, second);
      Serial.println(buf);
    } else {
      Serial.println("RTC was not saved(invalid date or communication error).");
    }
  }

  if (sleep_sec > 0) {
#ifdef ARDUINO_ARCH_ESP8266
    disconnect();  // disconnect from server

    rtcData.valid = 1;
    restart(sleep_sec);
#else
    Serial.println("Sleep is not implemented for this architecture yet");
#endif
  }

}

void connect(const char* additional_init_params = NULL)
{
  unsigned long retry_delay = 1;/*sec*/

  while (true) {
    bool success = false;
    if (rtcData.valid && rtcData.server_addr[0] != 0 && rtcData.server_port != 0) {
      Serial.print("Resuming connection to ");
      Serial.print(rtcData.server_addr);
      Serial.print(':');
      Serial.print(rtcData.server_port);
      Serial.println(".");
      success = connect(config.nodename, rtcData.server_addr, rtcData.server_port);
      // invalidate them once used
      rtcData.server_addr[0] = 0;
      rtcData.server_port = 0;
    } else {
      success = connect(config.nodename, config.servername, config.port);
    }
    if (success) {
      // save server address and port
      String remoteAddr = get_remote_address();
      if (remoteAddr.length() < sizeof(rtcData.server_addr)) {
        memcpy(rtcData.server_addr, remoteAddr.c_str(), remoteAddr.length() + 1);
      }
      rtcData.server_port = get_remote_port();

      String init_str = "INIT\tnodename:";
      init_str += config.nodename;
      if (additional_init_params) {
        init_str += '\t';
        init_str += additional_init_params;
      }
      send_message(init_str.c_str());
      break;
    }
    // else
    Serial.print("Connection failed. ");
    if (retry_delay >= 60) { // already tried >= 60sec
      Serial.println("System will restart after 60 seconds.");
      restart(60);
      return; // not reaching here
    }

    // else
    Serial.print("Performing retry (");
    Serial.print(retry_delay);
    Serial.println("sec)...");
    delay(retry_delay * 1000);
    retry_delay *= 2;
    if (retry_delay > 60) retry_delay = 60;
  }
}

#ifdef ARDUINO_ARCH_ESP8266
void preinit() {
  ESP8266WiFiClass::preinitWiFiOff();
}
#endif

static void restart(uint16_t delay_seconds)
{
  display.turnOff(); // Display OFF

  // calc crc data
  rtcData.crc = 0xffff;
  for (size_t i = 0; i < sizeof(rtcData) - sizeof(rtcData.crc); i++) {
    rtcData.crc = update_crc(rtcData.crc, ((uint8_t*)&rtcData)[i]);
  }

#if defined(ARDUINO_ARCH_ESP8266)
  ESP.rtcUserMemoryWrite(0, (uint32_t*)&rtcData, sizeof(rtcData));
  ESP.deepSleep(delay_seconds * 1000L * 1000L , WAKE_RF_DEFAULT);
#elif defined(ARDUINO_ARCH_ESP32)
  esp_sleep_enable_timer_wakeup(delay_seconds * 1000L * 1000L);
  esp_deep_sleep_start();
#else
  Serial.println("Sleep is not implemented for this architecture");
#endif
  delay(1000);
}

void setup() {
  Serial.begin(115200);

  display.begin(DISPLAY_I2C_ADDRESS);

  display.setTextColor(1/*Light pixel*/);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.display();

  Serial.println();
  Serial.print("Build date: ");
  Serial.println(__DATE__ " " __TIME__);

  Serial.print("CPU Freq: ");
  Serial.print(F_CPU / 1000000L);
  Serial.println("MHz");

#ifdef ARDUINO_ARCH_ESP8266
  Serial.print("NONOS SDK version: ");
  Serial.println(system_get_sdk_version());

  const rst_info *prst = ESP.getResetInfoPtr();
  reset_reason = prst->reason;
#endif
#ifdef ARDUINO_ARCH_ESP32
  Serial.print("ESP-IDF version: ");
  Serial.println(esp_get_idf_version());
  reset_reason = rtc_get_reset_reason(xPortGetCoreID());
#endif

  if (reset_reason == REASON_DEEP_SLEEP_AWAKE) {
#ifdef ARDUINO_ARCH_ESP8266
    // read rtc memory
    ESP.rtcUserMemoryRead(0, (uint32_t*)&rtcData, sizeof(rtcData));
#endif
    uint16_t crc = 0xffff;
    for (size_t i = 0; i < sizeof(rtcData) - sizeof(rtcData.crc); i++) {
      crc = update_crc(crc, ((uint8_t*)&rtcData)[i]);
    }
    if (crc != rtcData.crc) rtcData.valid = 0;
    else Serial.println("Resuming from deep sleep.");
  } else {
    rtcData.valid = 0;
  }

  // clear everything in rtcData when invalid
  if (!rtcData.valid) memset(&rtcData, 0, sizeof(rtcData));

  // read config from EEPROM
  Serial.write("Loading config from EEPROM...");
  EEPROM.begin(sizeof(config));
  EEPROM.get(0, config);
  EEPROM.end();
  uint8_t* p = (uint8_t*)&config;
  uint16_t crc = 0xffff;
  for (size_t i = 0; i < sizeof(config) - sizeof(config.crc); i++) {
    crc = update_crc(crc, p[i]);
  }
  if (crc != config.crc) {
    Serial.print("Checksum mismatch(expected=");
    Serial.print(crc);
    Serial.print(", actual=");
    Serial.print(config.crc);
    Serial.print("). entering command line only mode.\r\n# ");
    memset(&config, 0, sizeof(config));

    config.default_operation_mode = OPERATION_MODE_NORMAL;
    strcpy(config.nodename, DEFAULT_NODENAME);
    strcpy(config.ssid, "YOUR_ESSID");
    strcpy(config.key, "YOUR_WPA_KEY");
    strcpy(config.servername, DEFAULT_SERVERNAME);
    config.port = DEFAULT_PORT;
    config.slave_id = DEFAULT_SLAVE_ID;
    config.tft_rotation = DEFAULT_TFT_ROTATION;
    operation_mode = OPERATION_MODE_COMMAND_LINE_ONLY;

    display.clearDisplay();
    display.println("EEPROM not valid");
    display.println("-Command line only mode-");
    display.display();

    return;
  }
  // else

  operation_mode = config.default_operation_mode;

  // begin TFT early
  if (operation_mode == OPERATION_MODE_THERMOMETER) {
    early_setup_thermometer();
  } else if (operation_mode == OPERATION_MODE_EDOGAWA_MASTER) {
    early_setup_edogawa_master();
  }

  Serial.println("Done.");
  Serial.print("Operation mode: ");
  Serial.println(operation_mode);
  Serial.print("Nodename: ");
  Serial.println(config.nodename);
  Serial.print(("WiFi SSID: "));
  Serial.println(config.ssid);
  Serial.println("WiFi Password: *");
  Serial.print("Server(service) name: ");
  Serial.println(config.servername);
  Serial.print("Server port: ");
  Serial.println(config.port);

  if (!rtcData.valid) { // Not resuming from deep sleep
    // wait ESC key to enter command line only mode
    Serial.println("Send ESC to enter command line only mode...");

    unsigned long startTime = millis();
    while ((millis() - startTime) / 1000 < COMMAND_LINE_ONLY_MODE_WAIT_SECONDS) {
      if (!Serial.available()) continue;
      if (Serial.read() == 0x1b) {
        Serial.print("Entering command line only mode...\r\n# ");
        operation_mode = OPERATION_MODE_COMMAND_LINE_ONLY;

        display.clearDisplay();
        display.println("-Command line only mode-");
        display.display();

        return;
      }
    }
  }

  display.clearDisplay();
  display.println(config.nodename);
  display.print("SSID: ");
  display.println(config.ssid);
  display.display();

  if (operation_mode == OPERATION_MODE_NORMAL || operation_mode == OPERATION_MODE_NISETRACER || operation_mode == OPERATION_MODE_SLAVE) {
#if defined(ARDUINO_ARCH_ESP8266)
    modbus.begin(RS485_COMM_SOCKET, RS485_DE_SOCKET, RS485_RE_SOCKET, EPSOLAR_COMM_SPEED, MODBUS_TIMEOUT_MS);
#elif defined(ARDUINO_ARCH_ESP32)
    modbus.begin(UART_NUM_2, RS485_TX_SOCKET, RS485_RX_SOCKET, RS485_DE_SOCKET, RS485_RE_SOCKET, EPSOLAR_COMM_SPEED, MODBUS_TIMEOUT_MS);
#endif
#if defined(PW1_SW_SOCKET) && defined(PW1_LED_SOCKET)
    edogawaUnit1.begin(PW1_SW_SOCKET, PW1_LED_SOCKET);
#endif
#if defined(PW2_SW_SOCKET) && defined(PW2_LED_SOCKET)
    edogawaUnit2.begin(PW2_SW_SOCKET, PW2_LED_SOCKET);
#endif
  }

  if (operation_mode == OPERATION_MODE_NORMAL) {
    pinMode(GPIO_SOCKET, gpio_output_mode? OUTPUT : INPUT);

    EPSolarTracerInputRegister reg;
    if (epsolar.get_register(0x3200, 1, reg)) {
      set_battery_voltage_status((uint8_t)(reg.getWordValue(0) & 0x000f));
      Serial.print("Battery voltage status: ");
      Serial.println(get_battery_voltage_status_string());
    } else {
      Serial.println("Getting battery voltage status info failed!");
    }

    if (rtcData.valid && is_battery_low()) {
      rtcData.postpone_count += 1;
      if (rtcData.postpone_count <= MAX_POSTPONE_COUNT) {
        Serial.print("Postpone sending data due to low battery. Count=");
        Serial.println((int)rtcData.postpone_count);
        restart(LOW_BATTERY_SLEEP_SECONDS);
        return;
      }
      // else
      rtcData.postpone_count = 0;
    }

    if (!rtcData.valid) {
      EPSolarTracerDeviceInfo info;
      if (epsolar.get_device_info(info)) {
        Serial.print("Vendor: ");
        Serial.println(info.get_vendor_name());
        Serial.print("Product: ");
        Serial.println(info.get_product_code());
        Serial.print("Revision: ");
        Serial.println(info.get_revision());

        display.print("Vendor: ");
        display.println(info.get_vendor_name());
        display.print("Product: ");
        display.println(info.get_product_code());
        display.print("Revision: ");
        display.println(info.get_revision());
        display.display();
      } else {
        Serial.println("Getting charge controller device info failed!");
      }

      if (epsolar.get_register(0x9013/*Real Time Clock*/, 3, reg, 3)) {
        uint64_t rtc = reg.getRTCValue(0);
        char buf[128];
        sprintf(buf, "RTC: %lu %06lu", (uint32_t)(rtc / 1000000L), (uint32_t)(rtc % 1000000LL));
        Serial.println(buf);
        if (epsolar.get_register(0x9000/*battery type, battery capacity*/, 2, reg)) {
          const char* battery_type_str[] = { "User Defined", "Sealed", "GEL", "Flooded/LiFePO4" };
          int battery_type = reg.getIntValue(0);
          int battery_capacity = reg.getIntValue(2);
          sprintf(buf, "Battery type: %d(%s), %dAh", battery_type, battery_type_str[battery_type], battery_capacity);
          Serial.println(buf);
          if (battery_type == 0/*User defined*/ && epsolar.get_register(0x9003/* battery user settings */, 12, reg)) {
            sprintf(buf, "Boost Charging Voltage: %.2fV", reg.getFloatValue(8)/* * multiplier*/);
            Serial.println(buf);
            sprintf(buf, "Float Charging Voltage: %.2fV", reg.getFloatValue(10)/* * multiplier*/);
            Serial.println(buf);
            sprintf(buf, "Low Voltage Disconnect: %.2fV", reg.getFloatValue(20)/* * multiplier*/);
            Serial.println(buf);
          }
          if (epsolar.get_register(0x311d/*Battery real rated voltage*/, 1, reg)) {
            rtcData.battery_rated_voltage = (uint8_t)reg.getFloatValue(0);
            if (epsolar.get_register(0x9002/*Temperature compensation coefficient*/, 1, reg)) {
              rtcData.temperature_compensation_coefficient = (uint8_t)reg.getFloatValue(0);
            }
          }

          if (epsolar.put_register(0x903d/*Load controlling mode*/, (uint16_t)0)) {
            Serial.println("Load controlling mode set to 0(Manual)");
          }
          if (epsolar.put_register(0x906a/*Default load on/off in manual mode*/, (uint16_t)1)) {
            Serial.println("Default load on/off in manual mode set to 1(on)");
          }
        }
      } else {
        Serial.println("Getting charge controller settings failed!");
      }
    } // reset_reason != REASON_DEEP_SLEEP_AWAKE

    Serial.print("Battery real rated voltage: ");
    Serial.print((int)rtcData.battery_rated_voltage);
    Serial.println("V");

    Serial.print("Temperature compensation coefficient: ");
    Serial.print((int)rtcData.temperature_compensation_coefficient);
    Serial.println("mV/Cecelsius degree/2V");

    // check for INA219
    Wire.beginTransmission(INA219_ADDRESS);
    if (Wire.endTransmission() == 0) {
      Serial.println("INA219 current sensor found.");
      ina219.begin();
      ina219_started = true;
    }

  } // operation_mode == OPERATION_MODE_NORMAL

  if (operation_mode != OPERATION_MODE_THERMOMETER && operation_mode != OPERATION_MODE_NISETRACER && operation_mode != OPERATION_MODE_AMMETER && operation_mode != OPERATION_MODE_SLAVE && operation_mode != OPERATION_MODE_WATCHDOG) {
    Serial.print("Connecting to WiFi AP");
#ifdef ARDUINO_ARCH_ESP8266
    WiFi.forceSleepWake();
#endif
/*
#ifdef ARDUINO_ARCH_ESP32
    esp_wifi_set_max_tx_power(-128);
#endif
*/
    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);

    for (int i = 0; i < 2; i++) {
      if (i == 0) {
        if (rtcData.valid) {
          Serial.print("(Attempting connect to previously connected AP)");
          WiFi.begin(config.ssid, config.key, rtcData.channel, rtcData.bssid, true);
        } else continue;
      } else {
        // attempt to connect with ssid/key
        WiFi.begin(config.ssid, config.key);
      }
      char rot[] = {'|','/','-','\\', '\0'};
      char minus[] = {'-', '\0'};
      int irot = 0;
      unsigned long startTime = millis();
      while (WiFi.status() != WL_CONNECTED) {
        if (i == 0/*connect with stored bssid*/ && millis() - startTime > 5000/*5 seconds*/) {
          Serial.println("Abandon connecting previously connected AP");
          WiFi.disconnect();
          delay(10);
#ifdef ARDUINO_ARCH_ESP8266
          WiFi.forceSleepBegin();
          delay(10);
          WiFi.forceSleepWake();
          delay(10);
#endif
          break;
        } else if (millis() - startTime > 60000/* 1minute */) {
          if (is_battery_low()) {
            Serial.println("WiFi connection timeout. Due to low battery, next attempt will be after a while.");
            restart(LOW_BATTERY_SLEEP_SECONDS * MAX_POSTPONE_COUNT);
          } else {
            Serial.println("WiFi connection timeout. Sleeping 60 seconds...");
            restart(60);
          }
          continue; // not reaching here
        }
        display.setCursor(0, display.getCursorY());
        display.print("Connecting WiFi...");
        int16_t x, y;
        uint16_t w, h;
        display.getTextBounds(minus, display.getCursorX(), display.getCursorY(), &x, &y, &w, &h);
        display.fillRect(x, y, w, h, 0);
        display.print(rot[irot++]);
        if (!rot[irot]) irot = 0;
        display.display();
        delay(500);
        Serial.print(".");
        //Serial.println(WiFi.status());
      }

      if (WiFi.status() == WL_CONNECTED) break;
      else rtcData.valid = 0;
    }

    // save WiFi AP info to rtc data
    rtcData.channel = WiFi.channel();
    memcpy(rtcData.bssid, WiFi.BSSID(), sizeof(rtcData.bssid));

    Serial.println(" Connected.");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    display.clearDisplay();
    display.setCursor(0,0);
    display.println("WiFi connected.");
    display.print("IP address: ");
    display.println(WiFi.localIP());
    display.display();
  }

  if (operation_mode == OPERATION_MODE_NORMAL) {
    display.println("Connecting to server...");
    display.display();
    connect(reset_reason == REASON_DEEP_SLEEP_AWAKE? NULL : "boot:1");
    Serial.println(" Connected.");
    display.println("Connected.");
    display.display();
#ifdef ARDUINO_ARCH_ESP8266
    Serial.println("Entering modem sleep(MODEM_SLEEP_T)...");
    wifi_set_sleep_type(MODEM_SLEEP_T);
#elif defined ARDUINO_ARCH_ESP32
    Serial.println("Entering modem sleep(WIFI_PS_MAX_MODEM)...");
    esp_wifi_set_ps(WIFI_PS_MAX_MODEM) == ESP_OK;
#endif
  } else if (operation_mode == OPERATION_MODE_SERVER) {
    setup_server();
  } else if (operation_mode == OPERATION_MODE_NISETRACER) {
    setup_nisetracer();
  } else if (operation_mode == OPERATION_MODE_EDOGAWA_MASTER) {
    setup_edogawa_master();
  } else if (operation_mode == OPERATION_MODE_THERMOMETER) {
    setup_thermometer();
  } else if (operation_mode == OPERATION_MODE_AMMETER) {
    setup_ammeter();
  } else if (operation_mode == OPERATION_MODE_SLAVE) {
    setup_slave();
  } else if (operation_mode == OPERATION_MODE_WATCHDOG) {
    setup_watchdog();
  }
}

void loop_command_line()
{
  int available = Serial.available();
  if (available > 0) {
    for(int i = 0; i < available; i++) {
      char c = (char)Serial.read();
      if (c == '\r' || (c >= 0x20 && c < 0x7f)) {
        Serial.write(c); // echo back
        cmdline_buffer.concat(c);
      } else if (c == 0x7f/*DEL*/) {
        int len = cmdline_buffer.length();
        if (len > 0 && cmdline_buffer.charAt(len - 1) >= 0x20) {
          cmdline_buffer.remove(len - 1, 1);
          const char del_seq[] = { 0x08, 0x20, 0x08, 0x00 };
          Serial.write(del_seq);
        } else {
          Serial.write(0x07);
        }
      }
    }
    int line_size;
    while ((line_size = cmdline_buffer.indexOf('\r')) >= 0) {
      char line[line_size + 1];
      strncpy(line, cmdline_buffer.c_str(), line_size);
      line[line_size] = '\0';
      cmdline_buffer.remove(0, line_size + 1);
      Serial.println();
      if (!process_command_line(line)) {
        cmdline_buffer = "";
        return;
      }
      // else
      Serial.print("# ");
    }
  }
  delay(50);
}

bool getEPSolarValues(EPSolarValues& values)
{
  EPSolarTracerInputRegister reg;
  // TODO: apply https://github.com/wbrxcorp/solar/commit/a664b0577e7c9bbcac6448741c43e58ce6750e43
  if (!epsolar.get_register(0x3100, 6, reg)) return false;
  // else
  values.piv = reg.getFloatValue(0);
  values.pia = reg.getFloatValue(2);
  values.piw = reg.getDoubleValue(4);
  values.bv = reg.getFloatValue(8);
  values.poa = reg.getFloatValue(10);

  if (!epsolar.get_register(0x310e, 3, reg)) return false;
  //else
  values.load = reg.getDoubleValue(0);
  values.temp = reg.getFloatValue(4);

  if (!epsolar.get_register(0x311a, 1, reg)) return false;
  values.soc = reg.getPercentageValue(0);

  if (!epsolar.get_register(0x3201, 1, reg)) return false; // Charging equipment status
  //else
  values.cs = (reg.getWordValue(0) >> 2) & 0x0003;

  if (!epsolar.get_register(0x3304, 1, reg)) return false;
  //else
  values.lkwh = reg.getDoubleValue(0);

  if (!epsolar.get_register(0x330c, 1, reg)) return false;
  values.kwh = reg.getDoubleValue(0);
  if (!epsolar.get_register(0x0002, 1, reg)) return false; // Manual control the load
  // else
  values.pw = reg.getBoolValue(0)? 1 : 0;

  return true;
}

void loop_normal()
{
  // enter command line mode when ESC pressed
  if (Serial.available() && Serial.read() == 0x1b) {
    operation_mode = OPERATION_MODE_COMMAND_LINE;
    Serial.println("Entering command line mode. '?' to help, 'exit' to exit.");
    Serial.print("# ");

    display.clearDisplay();
    display.setCursor(0,0);
    display.println("-Command line mode-");
    display.display();

    return;
  }

  unsigned long current_time = millis();

  if (receive_message(process_message) > 0) last_message_received = current_time;

  if (last_message_sent > last_message_received && current_time - last_message_sent >= MESSAGE_TIMEOUT) {
    Serial.println("Message timeout. Disconnecting.");
    last_message_sent = 0L;
    disconnect();
  }

  if (!connected()) {
    Serial.println("Connection lost. Performing autoreconnect...");
    connect();
  }

  if (last_checked > 0L && current_time - last_checked <= CHECK_INTERVAL) {
    delay(100);
    return;
  }

  // else
  display.clearDisplay();
  display.setCursor(0,0);
  display.print("NODE ");
  display.println(config.nodename);

  String message;
  EPSolarValues values;
  if (getEPSolarValues(values)) {
    bool pw1 = edogawaUnit1? edogawaUnit1.is_power_on() : false;
    bool pw2 = edogawaUnit2? edogawaUnit2.is_power_on() : false;

    message = String("PV   ") + values.piw + "W\n"
      + "LOAD " + values.load + "W\n"
      + "BATT " + values.bv + "V\n" +
      + "TEMP " + values.temp + "deg.\n" +
      + "PW   " + (values.pw? "ON" : "OFF") + '\n'
      + "PW1  " + (pw1? "ON" : "OFF") + '\n';
    display.print(message);

    float btcv = 0.0f;
    if (rtcData.battery_rated_voltage && rtcData.temperature_compensation_coefficient && values.temp < 25.0f && values.piv >= values.bv && values.cs > 0) {
      btcv = 0.001 * rtcData.temperature_compensation_coefficient * (25.0f - values.temp) * (rtcData.battery_rated_voltage / 2);
    }

    message = String("DATA\tpiv:") + values.piv
      + "\tpia:" + values.pia
      + "\tpiw:" + values.piw
      + "\tbv:" + values.bv
      + "\tpoa:" + values.poa
      + "\tload:" + values.load
      + "\ttemp:" + values.temp
      + "\tlkwh:" + values.lkwh
      + "\tkwh:" + values.kwh
      + "\tpw:" + values.pw
      + "\tbtcv:" + btcv
      + "\tpw1:" + (pw1? 1 : 0)
      + "\tpw2:" + (pw2? 1 : 0)
      + "\trssi:" + (WiFi.RSSI())
      + "\tcs:" + values.cs
      + "\tsoc:" + (int)values.soc
      + "\tgpio:" + (gpio_output_mode? (gpio_output_value? 1 : 0) : (digitalRead(GPIO_SOCKET) == HIGH? 1: 0));

      if (ina219_started) {
        float aiw = ina219.getPower_mW() / 1000.0;
        message += String("\taia:") + ina219.getCurrent_mA() / 1000.0
          + "\taiw:" + aiw
          + "\taiv:" + (ina219.getBusVoltage_V() + ina219.getShuntVoltage_mV() / 1000.0);
        display.print(String("AUX  ") + aiw + "W\n");
      }
  } else {
    display.print("!Controller disconnected!");

    message = "NODATA";
  }

  display.display();

  last_checked = current_time;
  if (last_message_received < last_message_sent) return;
  // else
  if (last_reported > 0L && (current_time - last_reported <= REPORT_INTERVAL)
    && fabs(last_bv - values.bv) < last_bv * 0.01) return;

  // else
  if (session_id[0]) {
    message += "\tsession:";
    message += session_id;
  }

  message += "\tnodename:";
  message += config.nodename;
  send_message(message.c_str());
  last_bv = values.bv;
  last_message_sent = last_reported = current_time;

  if (is_battery_low()) {
    disconnect();  // disconnect from server
    rtcData.postpone_count = 1;
    rtcData.valid = 1;
    Serial.println("Sleep awhile due to low battery.");
    restart(LOW_BATTERY_SLEEP_SECONDS);
  }
}

void loop()
{
  switch (operation_mode) {
    case OPERATION_MODE_NORMAL:
      loop_normal();
      break;
    case OPERATION_MODE_COMMAND_LINE:
    case OPERATION_MODE_COMMAND_LINE_ONLY:
      loop_command_line();
      break;
    case OPERATION_MODE_SERVER:
      loop_server();
      break;
    case OPERATION_MODE_NISETRACER:
      loop_nisetracer();
      break;
    case OPERATION_MODE_EDOGAWA_MASTER:
      loop_edogawa_master();
      break;
    case OPERATION_MODE_THERMOMETER:
      loop_thermometer();
      break;
    case OPERATION_MODE_AMMETER:
      loop_ammeter();
      break;
    case OPERATION_MODE_SLAVE:
      loop_slave();
      break;
    case OPERATION_MODE_WATCHDOG:
      loop_watchdog();
      break;
    default:
      break;
  }
}
