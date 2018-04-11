// arduino --upload --board espressif:esp32:mhetesp32minikit:FlashFreq=80,UploadSpeed=921600 --port /dev/ttyUSB0 .
// arduino --upload --board esp8266com:esp8266:d1_mini:CpuFrequency=80,FlashSize=4M1M,UploadSpeed=921600 --port /dev/ttyUSB0 .
#include <EEPROM.h>
#ifdef ARDUINO_ARCH_ESP32
#include <WiFi.h>
#include <ESPmDNS.h>
#include <esp_wifi.h>
#elif ARDUINO_ARCH_ESP8266
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <user_interface.h>
#endif

#ifdef ARDUINO_ARCH_ESP32
  // map MH-ET ESP32 Mini Kit's pins to D1 mini's ones
  #define D3 17
  #define D4 16
  #define D5 18
  #define D6 19
  #define D7 23
  #define D8 5
  #define LED_BUILTIN 2
#endif

#ifdef ARDUINO_ARCH_ESP8266
  #undef LED_BUILTIN
#endif

#define RS485_TX_SOCKET D3  // ESP8266 IO0
#define RS485_RX_SOCKET D4  // ESP8266 IO2
#define RS485_RTS_SOCKET D7 // ESP8266 IO13
#define PW1_SW_SOCKET D5    // ESP8266 IO14
#define PW1_LED_SOCKET D6   // ESP8266 IO12

#define REPORT_INTERVAL 5000
#define MESSAGE_TIMEOUT 10000
#define COMMAND_LINE_ONLY_MODE_WAIT_SECONDS 3

const char* DEFAULT_NODENAME = "kennel01";
const char* DEFAULT_SERVERNAME = "_solar._tcp";
const uint16_t DEFAULT_PORT = 29574; // default server port number

#ifdef ARDUINO_ARCH_ESP32
  HardwareSerial RS485(1);  // Use UART1 (need to change TX/RX pins)
#elif ARDUINO_ARCH_ESP8266
  SoftwareSerial RS485(RS485_RX_SOCKET, RS485_TX_SOCKET, false, 256);
#endif

#include "command_line.h"

#include "Adafruit_SSD1306.h"
Adafruit_SSD1306 display(-1);

#include "logo.h"

#include "globals.h"

class LineBuffer {
  String buf;
  char terminator = '\n';
public:
  LineBuffer() {;}
  LineBuffer(char _terminator) : terminator(_terminator) { ; }
  bool push(char c) {
    buf += c;
    return this->get_line_size() >= 0;
  }
  bool push(const char* str) {
    buf += str;
    return this->get_line_size() >= 0;
  }
  bool push(const uint8_t* data, size_t size) {
    for (int i = 0; i < size; i++) {
      buf.concat((char)data[i]);
    }
    return this->get_line_size() >= 0;
  }
  int get_line_size() const {
    return buf.indexOf(terminator);
  }
  bool pop(char* dst, size_t dstsize) {
    if (dstsize < 1) return false;
    int line_size = this->get_line_size();
    if (line_size < 0) return false;

    int copy_size = std::min(line_size, (int)dstsize - 1);
    strncpy(dst, buf.c_str(), copy_size);
    dst[copy_size] = '\0';

    buf.remove(0, line_size + 1);
    return true;
  }
  void clear() { buf = ""; }
};

LineBuffer receive_buffer;
LineBuffer cmdline_buffer('\r'/*cu sends \r instead of \n*/);

WiFiClient tcp_client;

unsigned long last_report_time = 0;
char session_id[48] = "";
uint8_t battery_rated_voltage = 0; // 12 or 24(V)
uint8_t temperature_compensation_coefficient = 0; // 0-9(mV)
#ifdef ARDUINO_ARCH_ESP8266
  bool mdns_started = false;
#endif

void connect()
{
  unsigned long retry_delay = 1;/*sec*/
  int servername_len = strlen(config.servername);

  while (true) {
    int connected = 0;

#ifdef ARDUINO_ARCH_ESP32
    String _tmp_hostname = "nodexxx";
    _tmp_hostname += config.nodename;
    const char* tmp_hostname = _tmp_hostname.c_str();
    if (servername_len > 6 && strcmp(config.servername + servername_len - 6, ".local") == 0) {
      Serial.printf("Querying hostname '%s' via mDNS...", config.servername);
      char servername_without_suffix[servername_len - 5];
      strncpy(servername_without_suffix, config.servername, servername_len - 6);
      servername_without_suffix[servername_len - 6] = '\0';

      if (!mdns_init()) {
        String _tmp_hostname = "nodexxx";
        if (!mdns_hostname_set(tmp_hostname)) {
          struct ip4_addr addr;
          addr.addr = 0;
          esp_err_t err = mdns_query_a(servername_without_suffix, 5000,  &addr);
          if (!err) {
              IPAddress addr2 = IPAddress(addr.addr);
              Serial.print("Found. Connecting to ");
              Serial.print(addr2);
              Serial.print(':');
              Serial.print(config.port);
              Serial.print("...");

              connected = tcp_client.connect(addr2, config.port);
          } else {
            Serial.println("Failed to obtain server IP address via mDNS.");
          }
        } else {
          Serial.println("Failed to set mDNS hostname.");
        }
        mdns_free();
      } else {
        Serial.println("Failed to start mDNS.");
      }
    } else
#endif
    if (config.servername[0] == '_' && strcmp(config.servername + servername_len - 5, "._tcp") == 0) {
      char servicename[servername_len - 4];
      strncpy(servicename, config.servername, servername_len - 5);
      servicename[servername_len - 5] = '\0';
      Serial.printf("Querying service %s._tcp via mDNS...", servicename);

#ifdef ARDUINO_ARCH_ESP32
      if (MDNS.begin(tmp_hostname)) {
        mdns_result_t * results;
        esp_err_t err = mdns_query_ptr(servicename, "_tcp", 5000, 20,  &results);
        if (err) {
          Serial.println("Error.");
        } else if (!results) {
          Serial.println("Not Found.");
        } else {
          mdns_ip_addr_t * addr = results->addr;
          while(addr){
            if(addr->addr.type == MDNS_IP_PROTOCOL_V4) break;
            addr = addr->next;
          }
          if (addr) {
            IPAddress addr2 = IPAddress(addr->addr.u_addr.ip4.addr);
            Serial.print("Found. Connecting to ");
            Serial.print(addr2);
            Serial.print(':');
            Serial.print(results->port);
            Serial.print("...");
            connected = tcp_client.connect(addr2, results->port);
          } else {
            Serial.println("No IPv4 address attatched to the service.");
          }
          mdns_query_results_free(results);
        }
        MDNS.end();
      }
#elif ARDUINO_ARCH_ESP8266
      if (mdns_started || MDNS.begin(wifi_station_get_hostname())) {
        if (mdns_started) MDNS.update();
        else mdns_started = true;
        if (MDNS.queryService((const char*)(servicename + 1), "tcp") > 0) {
          IPAddress addr = MDNS.IP(0);
          uint16_t port = MDNS.port(0);
          Serial.print("Found. Connecting to ");
          Serial.print(addr);
          Serial.print(':');
          Serial.print(port);
          Serial.print("...");
          connected = tcp_client.connect(addr, port);
        } else {
          Serial.println("Not Found.");
        }
      }
#endif
      else {
        Serial.println("Failed to start mDNS.");
      }
    } else {
      Serial.print("Connecting to ");
      Serial.print(config.servername);
      Serial.print(':');
      Serial.print(config.port);
      Serial.print("...");
      connected = tcp_client.connect(config.servername, config.port);
    }

    if (connected) {
      Serial.println("Connected.");
      char buf[64];
      sprintf(buf, "INIT\tnodename:%s", config.nodename);
      send_message(buf);
      return;
    }
    // else
    Serial.print("Connection failed. Performing retry (");
    Serial.print(retry_delay);
    Serial.println("sec)...");
    delay(retry_delay * 1000);
    retry_delay *= 2;
    if (retry_delay > 60) retry_delay = 60;
  }
}

bool send_message(const char* message)
{
  tcp_client.write(message, strlen(message));
  tcp_client.write('\n');

  if (debug_mode) {
    Serial.println(message);
  }

  while (true) { // wait for response
    int available;
    unsigned long t = millis();
    while ((available = tcp_client.available()) == 0) {
      delay(100);
      if (millis() > t + MESSAGE_TIMEOUT) {
        Serial.println("Message timeout.");
        return false;
      }
    }
    uint8_t buf[available];
    int r = tcp_client.readBytes(buf, available);
    if (receive_buffer.push(buf, r)) {
      int line_size;
      while ((line_size = receive_buffer.get_line_size()) >= 0) {
        char line[line_size + 1];
        receive_buffer.pop(line, line_size + 1);
        process_message(line);
      }
      return true;
    }
  }
}

void process_message(const char* message)
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
        if (edogawaUnit.is_power_on()) edogawaUnit.power_off(); // atx power off
        epsolar.load_on(false);
      } else if (pw == 1) {
        Serial.println("Power ON");
        epsolar.load_on(true);
      }
    } else if (strcmp(key, "pw1") == 0 && (isdigit(value[0]) || value[0] == '-')) { // atx power
      int pw1 = atoi(value);
      bool pw1_on = edogawaUnit.is_power_on();
      if (pw1_on && pw1 == 0) { // power off
        Serial.println("Power1 OFF");
        edogawaUnit.power_off();
      } else if (!pw1_on && pw1 == 1) { // power on
        Serial.println("Power1 ON");
        epsolar.load_on(true); // main power on first
        edogawaUnit.power_on();
      }
    }
  }

  if (date > 20170101L && time >= 0) {
    uint16_t data[3];
    uint16_t year = date / 10000 - 2000;
    uint16_t month = date % 10000 / 100;
    uint16_t day = date % 100;
    uint16_t hour = time / 10000;
    uint16_t minute = time % 10000 / 100;
    uint16_t second = time % 100;

    if (year < 100 && month > 0 && month < 13 && day > 0 && day < 32 && hour < 24 && minute < 60 && second < 60) {
      data[0/*0x9013*/] = minute << 8 | second;
      data[1/*0x9014*/] = day << 8 | hour;
      data[2/*0x9015*/] = year << 8 | month;
      epsolar.put_registers(0x9013/*Real Time Clock*/, data, 3);
      char buf[32];
      sprintf(buf, "Date saved: 20%02u-%02u-%02u %02u:%02u:%02u", year, month, day, hour, minute, second);
      Serial.println(buf);
    }
  }
}

void setup() {
  Serial.begin(115200);
#ifdef LED_BUILTIN
  pinMode(LED_BUILTIN, OUTPUT);
#endif

  display.begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS);

  display.clearDisplay();
  display.drawXBitmap(0, 0, logo_bits, 128, 64, 1);
  display.display();

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

    strcpy(config.nodename, DEFAULT_NODENAME);
    strcpy(config.ssid, "YOUR_ESSID");
    strcpy(config.key, "YOUR_WPA_KEY");
    strcpy(config.servername, DEFAULT_SERVERNAME);
    config.port = DEFAULT_PORT;
    operation_mode = OPERATION_MODE_COMMAND_LINE_ONLY;

    display.println("EEPROM not valid");
    display.println("-Command line only mode-");
    display.display();

    return;
  }

  Serial.println("Done.");
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

      display.println("-Command line only mode-");
      display.display();

      return;
    }
  }

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.println(config.nodename);
  display.printf("SSID: %s\n", config.ssid);
  display.display();

  edogawaUnit.begin(PW1_SW_SOCKET, PW1_LED_SOCKET);

#ifdef ARDUINO_ARCH_ESP32
  RS485.begin(EPSOLAR_COMM_SPEED, SERIAL_8N1, RS485_RX_SOCKET, RS485_TX_SOCKET); // USE 16/17 pins originally assigned to UART2
#elif ARDUINO_ARCH_ESP8266
  RS485.begin(EPSOLAR_COMM_SPEED);
#endif
  epsolar.begin(&RS485, RS485_RTS_SOCKET);
  EPSolarTracerDeviceInfo info;
  if (epsolar.get_device_info(info)) {
    Serial.print("Vendor: ");
    Serial.println(info.get_vendor_name());
    Serial.print("Product: ");
    Serial.println(info.get_product_code());
    Serial.print("Revision: ");
    Serial.println(info.get_revision());

    display.printf("Vendor: %s\n", info.get_vendor_name());
    display.printf("Product: %s\n", info.get_product_code());
    display.printf("Revision: %s\n", info.get_revision());
    display.display();
  } else {
    Serial.println("Getting charge controller device info failed!");
  }

  EPSolarTracerInputRegister reg;
  if (epsolar.get_register(0x9013/*Real Time Clock*/, 3, reg, 3)) {
    uint64_t rtc = reg.getRTCValue(0);
    char buf[32];
    sprintf(buf, "RTC: %lu %06lu", (uint32_t)(rtc / 1000000L), (uint32_t)(rtc % 1000000LL));
    Serial.println(buf);
    if (epsolar.get_register(0x9000/*battery type, battery capacity*/, 2, reg)) {
      const char* battery_type_str[] = { "User Defined", "Sealed", "GEL", "Flooded" };
      int battery_type = reg.getIntValue(0);
      int battery_capacity = reg.getIntValue(2);
      sprintf(buf, "Battery type: %d(", battery_type);
      strcat(buf, battery_type_str[battery_type]);
      sprintf(buf + strlen(buf), "), %dAh", battery_capacity);
      Serial.println(buf);
      if (epsolar.get_register(0x311d/*Battery real rated voltage*/, 1, reg)) {
        battery_rated_voltage = (uint8_t)reg.getFloatValue(0);
        Serial.print("Battery real rated voltage: ");
        Serial.print((int)battery_rated_voltage);
        Serial.println("V");
        if (epsolar.get_register(0x9002/*Temperature compensation coefficient*/, 1, reg)) {
          temperature_compensation_coefficient = (uint8_t)reg.getFloatValue(0);
          Serial.print("Temperature compensation coefficient: ");
          Serial.print((int)temperature_compensation_coefficient);
          Serial.println("mV/Cecelsius degree/2V");
          if (epsolar.get_register(0x0006/*Force the load on/off*/, 1, reg)) {
            Serial.print("Force the load on/off: ");
            Serial.println(reg.getBoolValue(0)? "on" : "off(used for test)");
          }
        }
      }
    }
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

  Serial.print("Connecting to WiFi AP");
  display.println("Connecting WiFi...");
  display.display();
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(config.ssid, config.key);
  while (WiFi.status() != WL_CONNECTED) {
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

  display.println("Connecting to server...");
  display.display();
  connect();
  display.println("Connected.");
  display.display();

#ifdef ARDUINO_ARCH_ESP32
  esp_wifi_set_ps(WIFI_PS_MAX_MODEM) == ESP_OK;
#elif ARDUINO_ARCH_ESP8266
  wifi_set_sleep_type(MODEM_SLEEP_T);
#endif
}

void loop_command_line()
{
  int available = Serial.available();
  if (available > 0) {
    uint8_t buf[available];
    size_t len = Serial.readBytes(buf, available);
    Serial.write(buf, len); // echo back
    if (cmdline_buffer.push(buf, len)) {
      int line_size; // can't be size_t as it can be negative value
      while ((line_size = cmdline_buffer.get_line_size()) >= 0) {
        char line[line_size + 1];
        cmdline_buffer.pop(line, line_size + 1);
        Serial.println();
        if (!process_command_line(line)) {
          cmdline_buffer.clear();
          return;
        }
        // else
        Serial.print("# ");
      }
    }
  }
  delay(50);
}

void loop_normal()
{
  // enter command line mode when ESC presses
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

  display.clearDisplay();
  display.setCursor(0,0);
  display.printf("NODE %s\n", config.nodename);

  unsigned long current_time = millis();
  if (current_time - last_report_time >= REPORT_INTERVAL) {
    if (!tcp_client.connected()) {
      Serial.println("TCP session disconnected. Recovering.");
      connect();
    }

    EPSolarTracerInputRegister reg;
    float piv,pia,bv,poa;
    double piw;
    double load;
    float temp;
    int cs;
    double lkwh;
    double kwh;
    int pw;

    bool success = false;

    // TODO: apply https://github.com/wbrxcorp/solar/commit/a664b0577e7c9bbcac6448741c43e58ce6750e43
    if (epsolar.get_register(0x3100, 6, reg)) {
      piv = reg.getFloatValue(0);
      pia = reg.getFloatValue(2);
      piw = reg.getDoubleValue(4);
      bv = reg.getFloatValue(8);
      poa = reg.getFloatValue(10);
      if (epsolar.get_register(0x310e, 3, reg)) {
        load = reg.getDoubleValue(0);
        temp = reg.getFloatValue(4);
        if (epsolar.get_register(0x3201, 1, reg)) { // Charging equipment status
          cs = (reg.getWordValue(0) >> 2) & 0x0003;
          if (epsolar.get_register(0x3304, 1, reg)) {
            lkwh = reg.getDoubleValue(0);
            if (epsolar.get_register(0x330c, 1, reg)) {
              kwh = reg.getDoubleValue(0);
              if (epsolar.get_register(0x0002, 1, reg)) { // Manual control the load
                pw = reg.getBoolValue(0)? 1 : 0;
                success = true;
              }
            }
          }
        }
      }
    }

    String message;

    if (success) {
      bool pw1 = edogawaUnit.is_power_on();

      message = String("PV   ") + piw + "W\n"
        + "LOAD " + load + "W\n"
        + "BATT " + bv + "V\n" +
        + "TEMP " + temp + "deg.\n" +
        + "PW   " + (pw? "ON" : "OFF") + '\n'
        + "PW1  " + (pw1? "ON" : "OFF") + '\n';
      display.print(message);

      float btcv = 0.0f;
      if (battery_rated_voltage && temperature_compensation_coefficient && temp < 25.0f && piv >= bv && cs > 0) {
        btcv = 0.001 * temperature_compensation_coefficient * (25.0f - temp) * (battery_rated_voltage / 2);
      }

      message = String("DATA\tpiv:") + piv
        + "\tpia:" + pia
        + "\tpiw:" + piw
        + "\tbv:" + bv
        + "\tpoa:" + poa
        + "\tload:" + load
        + "\ttemp:" + temp
        + "\tlkwh:" + lkwh
        + "\tkwh:" + kwh
        + "\tpw:" + pw
        + "\tbtcv:" + btcv
        + "\tpw1:" + (pw1? 1 : 0);
    } else {
      display.print("!Controller disconnected!");

      message = "NODATA";
    }

    display.display();

    if (session_id[0]) {
      message += "\tsession:";
      message += session_id;
    }

    message += "\tnodename:";
    message += config.nodename;

    while (!send_message(message.c_str())) {
      // Perform autoreconnect when something fails
      Serial.println("Connection error. performing autoreconnect...");
      delay(970);
      connect();
    }

    last_report_time = current_time;
  }
#ifdef LED_BUILTIN
  digitalWrite(LED_BUILTIN, current_time / 1000 % 2 == 0);
#endif
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
    default:
      break;
  }
}
