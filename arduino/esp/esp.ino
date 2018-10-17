// arduino --upload --board espressif:esp32:mhetesp32minikit:FlashFreq=80,UploadSpeed=921600 --port /dev/ttyUSB0 .
// arduino --upload --board esp8266com:esp8266:d1_mini:xtal=80,eesz=4M1M,baud=460800 --port /dev/ttyUSB0 .
// arduino --upload --board arduino:avr:mega:cpu=atmega2560 --port /dev/ttyACM0 .
#include <EEPROM.h>
#ifdef ARDUINO_ARCH_ESP32
#include <WiFi.h>
#include <esp_wifi.h>
#elif ARDUINO_ARCH_ESP8266
#include <ESP8266WiFi.h>
#include <user_interface.h>
#include "OneWireHalfDuplexSoftwareSerial.h"
#endif

#ifdef ARDUINO_ARCH_ESP8266
  #define RS485_RTS_SOCKET 2
  #define PW1_SW_SOCKET 14    // ESP8266 IO14 D1 mini D5
  #define PW1_LED_SOCKET 12   // ESP8266 IO12 D1 mini D6
  #define PW2_SW_SOCKET 15    // ESP8266 IO15(10k pull down) D1 mini D8
  #define PW2_LED_SOCKET 13   // ESP8266 IO13 D1 mini D7
  #undef LED_BUILTIN
#elif ARDUINO_ARCH_ESP32
  #define RS485_TX_SOCKET 17
  #define RS485_RX_SOCKET 16
  #define RS485_RTS_SOCKET 26
  #define PW1_SW_SOCKET 18
  #define PW1_LED_SOCKET 19
  #define PW2_SW_SOCKET 5
  #define PW2_LED_SOCKET 23
  #define LED_BUILTIN 2
#elif ARDUINO_AVR_MEGA2560
  #define RS485_TX_SOCKET 18
  #define RS485_RX_SOCKET 19
  #define RS485_RTS_SOCKET 2
  #define PW1_SW_SOCKET 3
  #define PW1_LED_SOCKET 4
  #define PW2_SW_SOCKET 6
  #define PW2_LED_SOCKET 5
#endif

#define CHECK_INTERVAL 5000
#define REPORT_INTERVAL 5000
#define MESSAGE_TIMEOUT 10000
#define MODBUS_TIMEOUT 100
#define COMMAND_LINE_ONLY_MODE_WAIT_SECONDS 3

const char* DEFAULT_NODENAME = "kennel01";
const char* DEFAULT_SERVERNAME = "_solar._tcp";
const uint16_t DEFAULT_PORT = 29574; // default server port number

#ifdef ARDUINO_ARCH_ESP32
  HardwareSerial RS485(1);  // Use UART1 (need to change TX/RX pins)
#elif ARDUINO_ARCH_ESP8266
  OneWireHalfDuplexSoftwareSerial& RS485 = *OneWireHalfDuplexSoftwareSerial::getInstance(256);
#elif ARDUINO_AVR_MEGA2560
  HardwareSerial& RS485 = Serial1;
#endif

#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)
  #define HAVE_WIFI
#endif

#include "network.h"
#include "command_line.h"
#include "server.h"

#include "logo.h"

#include "globals.h"

String cmdline_buffer;

unsigned long last_message_sent = 0;
unsigned long last_message_received = 0;
unsigned long last_checked = 0;
unsigned long last_reported = 0;
char session_id[48] = "";
uint8_t battery_rated_voltage = 0; // 12 or 24(V)
uint8_t temperature_compensation_coefficient = 0; // 0-9(mV)

typedef struct strEPSolarValues {
  float piv,pia,bv,poa;
  double piw;
  double load;
  float temp;
  int cs;
  double lkwh;
  double kwh;
  int pw;
} EPSolarValues;

static void process_message(const char* message)
{
  Serial.print("Received: ");
  Serial.println(message);

  if (strlen(message) < 3 || strncmp(message, "OK\t", 3) != 0) return;
  // else
  const char* pt = message + 3;
  int32_t date = -1, time = -1;
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
    } else if (strcmp(key, "bt") == 0) {
      int battery_type = atoi(value);
      epsolar.put_register(0x9000/*Battery type*/, (uint16_t)battery_type);
      Serial.print("Battery type saved: ");
      Serial.println(battery_type);
    } else if (strcmp(key, "bc") == 0) {
      int battery_capacity= atoi(value);
      epsolar.put_register(0x9001/*Battery capacity*/, (uint16_t)battery_capacity);
      Serial.print("Battery capacity saved: ");
      Serial.print(battery_capacity);
      Serial.println("Ah");
    } else if (strcmp(key, "pw") == 0 && isdigit(value[0])) { // main power
      int pw = atoi(value);
      if (pw == 0) {
        Serial.println("Power OFF");
        if (edogawaUnit1.is_power_on()) edogawaUnit1.power_off(); // atx power off
        if (edogawaUnit2.is_power_on()) edogawaUnit2.power_off(); // atx power off
        epsolar.load_on(false);
      } else if (pw == 1) {
        Serial.println("Power ON");
        epsolar.load_on(true);
      }
    } else if (key[0] == 'p' && key[1] == 'w' && (key[2] == '1' || key[2] == '2') && (isdigit(value[0]) || value[0] == '-')) { // atx power
      EdogawaUnit& edogawaUnit = key[2] == '1'? edogawaUnit1 : edogawaUnit2;
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
  }

  if (date > 20170101L && time >= 0) {
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
      Serial.println("RTC did not saved(invalid date or communication error).");
    }
  }
}

void connect(const char* additional_init_params = NULL)
{
  unsigned long retry_delay = 1;/*sec*/

  while (true) {
    if (connect(config.nodename, config.servername, config.port)) {
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
    Serial.print("Connection failed. Performing retry (");
    Serial.print(retry_delay);
    Serial.println("sec)...");
    delay(retry_delay * 1000);
    retry_delay *= 2;
    if (retry_delay > 60) retry_delay = 60;
  }

  last_message_sent = millis();
}

void setup() {
  Serial.begin(115200);
#ifdef LED_BUILTIN
  pinMode(LED_BUILTIN, OUTPUT);
#endif

  display.begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS);

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.drawXBitmap(0, 0, logo_bits, 128, 64, 1);
  display.display();

  Serial.println();
  Serial.print("Build date: ");
  Serial.println(__DATE__ " " __TIME__);

  Serial.println("Half duplex communication mode: Single wire");

  // read config from EEPROM
  Serial.write("Loading config from EEPROM...");
#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)
  EEPROM.begin(sizeof(config));
#endif
  EEPROM.get(0, config);
#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)
  EEPROM.end();
#endif
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
    operation_mode = OPERATION_MODE_COMMAND_LINE_ONLY;

    display.clearDisplay();
    display.println("EEPROM not valid");
    display.println("-Command line only mode-");
    display.display();

    return;
  }
  // else

  operation_mode = config.default_operation_mode;

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

  display.clearDisplay();
  display.println(config.nodename);
  display.print("SSID: ");
  display.println(config.ssid);
  display.display();

  if (operation_mode == OPERATION_MODE_NORMAL) {
  #ifdef ARDUINO_ARCH_ESP3esp2
    RS485.begin(EPSOLAR_COMM_SPEED, SERIAL_8N1, RS485_RX_SOCKET, RS485_TX_SOCKET); // USE 16/17 pins originally assigned to UART2
  #elif ARDUINO_ARCH_ESP8266
    RS485.begin(EPSOLAR_COMM_SPEED);
  #endif
    RS485.setTimeout(MODBUS_TIMEOUT);
    epsolar.begin(&RS485, RS485_RTS_SOCKET);

    edogawaUnit1.begin(PW1_SW_SOCKET, PW1_LED_SOCKET);
    edogawaUnit2.begin(PW2_SW_SOCKET, PW2_LED_SOCKET);

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

    EPSolarTracerInputRegister reg;
    if (epsolar.get_register(0x9013/*Real Time Clock*/, 3, reg, 3)) {
      uint64_t rtc = reg.getRTCValue(0);
      char buf[128];
      sprintf(buf, "RTC: %lu %06lu", (uint32_t)(rtc / 1000000L), (uint32_t)(rtc % 1000000LL));
      Serial.println(buf);
      if (epsolar.get_register(0x9000/*battery type, battery capacity*/, 2, reg)) {
        const char* battery_type_str[] = { "User Defined", "Sealed", "GEL", "Flooded" };
        int battery_type = reg.getIntValue(0);
        int battery_capacity = reg.getIntValue(2);
        sprintf(buf, "Battery type: %d(%s), %dAh", battery_type, battery_type_str[battery_type], battery_capacity);
        Serial.println(buf);
        if (epsolar.get_register(0x311d/*Battery real rated voltage*/, 1, reg)) {
          battery_rated_voltage = (uint8_t)reg.getFloatValue(0);
          sprintf(buf, "Battery real rated voltage: %dV", (int)battery_rated_voltage);
          Serial.println(buf);
          if (epsolar.get_register(0x9002/*Temperature compensation coefficient*/, 1, reg)) {
            temperature_compensation_coefficient = (uint8_t)reg.getFloatValue(0);
            sprintf(buf, "Temperature compensation coefficient: %dmV/Cecelsius degree/2V", (int)temperature_compensation_coefficient);
            Serial.println(buf);
            if (epsolar.get_register(0x0006/*Force the load on/off*/, 1, reg)) {
              sprintf(buf, "Force the load on/off: %s", reg.getBoolValue(0)? "on" : "off(used for test)");
              Serial.println(buf);
            }
          }
        }
      }

      // lifpo4:
      //  9000=0(User)
      //  9002=0
      //  9003=15.68
      //  9004=14.6
      //  9005=14.6
      //  9006=14.4
      //  9007=14.4
      //  9008=13.6
      //  9009=13.2
      //  900a=12.4
      //  900b=12.2
      //  900c=12.0
      //  900d=11.0
      //  900e=10.8
      //  9067=1(12V) or 2(24V)

      if (epsolar.put_register(0x903d/*Load controlling mode*/, (uint16_t)0)) {
        Serial.println("Load controlling mode set to 0(Manual)");
      }
      if (epsolar.put_register(0x906a/*Default load on/off in manual mode*/, (uint16_t)1)) {
        Serial.println("Default load on/off in manual mode set to 1(on)");
      }
      if (epsolar.put_register(0x0006/*Force the load on/off*/, (uint16_t)0xff00)) {
        Serial.println("Force the load on/off set to 'on'");
      }
    } else {
      Serial.println("Getting charge controller settings failed!");
    }
  } // operation_mode == OPERATION_MODE_NORMAL

#ifdef HAVE_WIFI
  Serial.print("Connecting to WiFi AP");
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(config.ssid, config.key);
  char rot[] = {'|','/','-','\\', '\0'};
  char minus[] = {'-', '\0'};
  int irot = 0;
  while (WiFi.status() != WL_CONNECTED) {
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
  Serial.println(" Connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  display.clearDisplay();
  display.setCursor(0,0);
  display.println("WiFi connected.");
  display.print("IP address: ");
  display.println(WiFi.localIP());
  display.display();

  if (operation_mode == OPERATION_MODE_NORMAL) {
    display.println("Connecting to server...");
    display.display();
    connect("boot:1");
    Serial.println(" Connected.");
    display.println("Connected.");
    display.display();
#ifdef ARDUINO_ARCH_ESP32
      Serial.println("Entering modem sleep(WIFI_PS_MAX_MODEM)...");
      esp_wifi_set_ps(WIFI_PS_MAX_MODEM) == ESP_OK;
#elif ARDUINO_ARCH_ESP8266
      Serial.println("Entering modem sleep(MODEM_SLEEP_T)...");
      wifi_set_sleep_type(MODEM_SLEEP_T);
#endif
  } else if (operation_mode == OPERATION_MODE_SERVER) {
    setup_server();
  }

#endif // HAVE_WIFI
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
#ifdef LED_BUILTIN
  digitalWrite(LED_BUILTIN, current_time / 1000 % 2 == 0);
#endif

#ifdef HAVE_WIFI
  if (receive_message(process_message) > 0) last_message_received = current_time;

  if (last_message_sent > last_message_received && current_time - last_message_sent >= MESSAGE_TIMEOUT) {
    Serial.println("Message timeout. Disconnecting.");
    disconnect();
  }

  if (!connected()) {
    Serial.println("Connection lost. Performing autoreconnect...");
    connect();
    return;
  }
#endif

  if (current_time - last_checked <= CHECK_INTERVAL) return;

  // else
  display.clearDisplay();
  display.setCursor(0,0);
  display.print("NODE ");
  display.println(config.nodename);

  String message;
  EPSolarValues values;
  if (getEPSolarValues(values)) {
    bool pw1 = edogawaUnit1.is_power_on();
    bool pw2 = edogawaUnit2.is_power_on();

    message = String("PV   ") + values.piw + "W\n"
      + "LOAD " + values.load + "W\n"
      + "BATT " + values.bv + "V\n" +
      + "TEMP " + values.temp + "deg.\n" +
      + "PW   " + (values.pw? "ON" : "OFF") + '\n'
      + "PW1  " + (pw1? "ON" : "OFF") + '\n'
      + "PW2  " + (pw2? "ON" : "OFF") + '\n';
    display.print(message);

    float btcv = 0.0f;
    if (battery_rated_voltage && temperature_compensation_coefficient && values.temp < 25.0f && values.piv >= values.bv && values.cs > 0) {
      btcv = 0.001 * temperature_compensation_coefficient * (25.0f - values.temp) * (battery_rated_voltage / 2);
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
      + "\tpw2:" + (pw2? 1 : 0);
  } else {
    display.print("!Controller disconnected!");

    message = "NODATA";
  }

  display.display();

  last_checked = current_time;
  if (current_time - last_reported <= REPORT_INTERVAL || last_message_received < last_message_sent) return;

  // else
  if (session_id[0]) {
    message += "\tsession:";
    message += session_id;
  }

  message += "\tnodename:";
  message += config.nodename;
#ifdef HAVE_WIFI
  send_message(message.c_str());
#else
  Serial.println(message);
#endif
  last_message_sent = last_reported = current_time;
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
    default:
      break;
  }
}
