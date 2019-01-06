#include <Arduino.h>
#include <EEPROM.h>
#include <FS.h>

#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#include <SPIFFS.h>
#endif

#include "command_line.h"
#include "globals.h"
#include "crc.h"
#include "epsolar.h"


static EPSolar epsolar(modbus);

class LineParser {
  char* buf = NULL;
  size_t len = 0;
  int count = 0;

protected:
  inline bool is_whitespace(char c) { return (c == ' ' || c == '\t'); }
public:
  LineParser(const char* line) {
    while (is_whitespace(*line)) line++;
    this->len = strlen(line);
    this->buf = new char[this->len + 1];
    strcpy(this->buf, line);
    for (int i = 0; i < len; i++) {
      if (is_whitespace(this->buf[i])) this->buf[i] = '\0';
    }
    const char* pt = this->buf;
    while (pt - this->buf < len) {
      if (*pt) {
        this->count++;
        pt = strchr(pt, '\0');
      } else {
        while (pt - this->buf < len && *pt == '\0') pt++;
      }
    }
  }
  ~LineParser() {
    delete [] buf;
  }

  int get_count() const { return this->count; }

  const char* operator[](int index) const {
    if (index >= count) return NULL;
    // else
    const char* pt = this->buf;
    for (int i = 0; i < index; i++) {
      while (*pt) pt++;
      while (!*pt) pt++;
    }
    return pt;
  }
};

bool _exit(const LineParser& lineparser)
{
  if (operation_mode == OPERATION_MODE_COMMAND_LINE_ONLY) {
    Serial.println("This is 'command line only' mode. make sure wifi config is right and restart system.");
    return true;
  }
  // else
  operation_mode = config.default_operation_mode;
  Serial.println("Exitting command line mode.");
  return false;
}

bool defaultmode(const LineParser& lineparser)
{
  if (lineparser.get_count() < 2) {
    Serial.print("Current default operation mode is '");
    Serial.print(config.default_operation_mode);
    Serial.println("'.");
    return true;
  }
  // else
  int default_operation_mode = atoi(lineparser[1]);
  if (default_operation_mode < 0 || default_operation_mode > OPERATION_MODE_MAX) {
    Serial.println("Invalid operation mode number.");
    return true;
  }
  // else
  config.default_operation_mode = (uint8_t)default_operation_mode;
  Serial.print("Defaul operation mode set to ");
  Serial.print(default_operation_mode);
  Serial.println(". save and reboot the system to take effects.");
  return true;
}

bool nodename(const LineParser& lineparser)
{
  if (lineparser.get_count() < 2) {
    Serial.print("Current nodename is '");
    Serial.print(config.nodename);
    Serial.println("'.");
    return true;
  }
  // else
  // TODO: validation
  strncpy(config.nodename, lineparser[1], sizeof(config.nodename));
  config.nodename[sizeof(config.nodename) - 1] = '\0'; // ensure null terminated as strncpy may not put it on tail
  Serial.print("Nodename set to '");
  Serial.print(config.nodename);
  Serial.println("'. save and reboot the system to take effects.");
  return true;
}

bool ssid(const LineParser& lineparser)
{
  if (lineparser.get_count() < 2) {
    Serial.print("Current SSID is '");
    Serial.print(config.ssid);
    Serial.println("'.");
    return true;
  }
  // else
  strncpy(config.ssid, lineparser[1], sizeof(config.ssid));
  config.ssid[sizeof(config.ssid) - 1] = '\0';
  Serial.print("SSID set to '");
  Serial.print(config.ssid);
  Serial.println("'. save and reboot the system to take effects.");
  return true;
}

bool key(const LineParser& lineparser)
{
  if (lineparser.get_count() < 2) {
    Serial.print("Current WPA Key is '");
    Serial.print(config.key);
    Serial.println("'.");
    return true;
  }
  // else
  strncpy(config.key, lineparser[1], sizeof(config.key));
  config.key[sizeof(config.key) - 1] = '\0';
  Serial.print("WPA Key set to '");
  Serial.print(config.key);
  Serial.println("'. save and reboot the system to take effects.");
}

bool servername(const LineParser& lineparser)
{
  if (lineparser.get_count() < 2) {
    Serial.print("Current Server name is '");
    Serial.print(config.servername);
    Serial.println("'.");
    return true;
  }
  // else
  strncpy(config.servername, lineparser[1], sizeof(config.servername));
  config.servername[sizeof(config.servername) - 1] = '\0';
  Serial.print("Server name set to '");
  Serial.print(config.servername);
  Serial.println("'. save and reboot the system to take effects.");
  return true;
}

bool port(const LineParser& lineparser)
{
  if (lineparser.get_count() < 2) {
    Serial.print("Current port is ");
    Serial.print(config.port);
    Serial.println('.');
    return true;
  }
  // else
  long port = atol(lineparser[1]);
  if (port < 1 || port > 65535) {
    Serial.println("Invalid port number.");
    return true;
  }
  // else
  config.port = (uint16_t)port;
  Serial.print("Server port set to ");
  Serial.print(port);
  Serial.println(". save and reboot the system to take effects.");
  return true;
}

bool save(const LineParser& lineparser)
{
  uint8_t* p = (uint8_t*)&config;
  config.crc = 0xffff;
  for (size_t i = 0; i < sizeof(config) - sizeof(config.crc); i++) {
    config.crc = update_crc(config.crc, p[i]);
  }
  Serial.print("Writing config to EEPROM...");
#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)
  EEPROM.begin(sizeof(config));
#endif
  EEPROM.put(0, config);
#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)
  EEPROM.commit();
  EEPROM.end();
#endif

  Serial.println("Done.");
  return true;
}

bool pw(const LineParser& lineparser)
{
  if (lineparser.get_count() < 2) {
    EPSolarTracerInputRegister reg;
    if (epsolar.get_register(0x0002, 1, reg)) { // Manual control the load
      Serial.println(reg.getBoolValue(0)? "on" : "off");
    } else {
      Serial.println("No response from charge controller.");
    }
    return true;
  }
  // else
  if (!isdigit(lineparser[1][0])) {
    Serial.println("Invalid parameter. (must be 1 or 0)");
    return true;
  }

  int _pw = atoi(lineparser[1]);
  if (_pw == 0) {
    Serial.println("Turning main power OFF");
    epsolar.load_on(false);
  } else if (_pw == 1) {
    Serial.println("Turning main power ON");
    epsolar.load_on(true);
  }

  return true;
}

bool pwX(const LineParser& lineparser)
{
  if (strlen(lineparser[0]) < 3) return true; //strange

  char X = lineparser[0][2];
  if (X != '1' && X != '2') return true;  // out of range

  EdogawaUnit& edogawaUnit = X == '1'? edogawaUnit1 : edogawaUnit2;

  if (lineparser.get_count() < 2) {
    Serial.print("pw");
    Serial.print(X);
    Serial.print(" is ");
    Serial.println(edogawaUnit.is_power_on()? "on" : "off");
    return true;
  }

  // else
  if (!isdigit(lineparser[1][0])) {
    Serial.println("Invalid parameter. (must be 1 or 0)");
    return true;
  }

  int pw = atoi(lineparser[1]);
  if (pw == 0) {
    Serial.print("Turning power");
    Serial.print(X);
    Serial.println(" OFF");
    edogawaUnit.power_off(); // atx power off
  } else if (pw == 1) {
    Serial.print("Turning power");
    Serial.print(X);
    Serial.println(" ON");
    epsolar.load_on(true); // main power on first
    edogawaUnit.power_on();
  }

  return true;
}

bool debug(const LineParser& lineparser)
{
  if (lineparser.get_count() < 2) {
    Serial.print("Debug mode is ");
    Serial.println(debug_mode? "on" : "off");
    return true;
  }
  // else
  if (!isdigit(lineparser[1][0])) {
    Serial.println("Invalid parameter. (must be 1 or 0)");
    return true;
  }

  int _debug = atoi(lineparser[1]);
  if (_debug == 0) {
    debug_mode = false;
    Serial.println("Debug mode set to OFF");
  } else if (_debug == 1) {
    debug_mode = true;
    Serial.println("Debug mode set to ON");
  }
  return true;
}

bool device_info(const LineParser& lineparser)
{
  EPSolarTracerDeviceInfo info;
  if (epsolar.get_device_info(info)) {
    Serial.print("Vendor: ");
    Serial.println(info.get_vendor_name());
    Serial.print("Product: ");
    Serial.println(info.get_product_code());
    Serial.print("Revision: ");
    Serial.println(info.get_revision());
  } else {
    Serial.println("Getting charge controller device info failed!");
  }
  return true;
}

bool uptime(const LineParser& lineparser)
{
  Serial.print("Uptime(in millis): ");
  Serial.println(millis());
  return true;
}

bool reboot(const LineParser& lineparser)
{
#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)
  Serial.println("Rebooting...");
  display.turnOff();
  ESP.restart();
#else
  Serial.println("Reboot not implemented...");
#endif
  return true; // never reaches here
}

bool read9022(const LineParser& lineparser)
{
  EPSolarTracerInputRegister reg;
  if (epsolar.get_register(0x9022, 2, reg)) {
    //Serial.print("0x900f="); Serial.println(reg.getIntValue(0));
    Serial.print("0x9022="); Serial.println(reg.getIntValue(0));
    Serial.print("0x9023="); Serial.println(reg.getIntValue(1));
  } else {
    Serial.println("Failed");
  }
  return true;
}

bool r9010(const LineParser& lineparser)
{
  if (lineparser.get_count() > 1) {
    if (!isdigit(lineparser[1][0])) {
      Serial.println("Invalid parameter. (must be a number)");
      return true;
    }

    uint16_t val = (uint16_t)atoi(lineparser[1]);

    if (epsolar.put_register(0x9010, val)) {
      Serial.println("Success.");
    } else {
      Serial.println("Failed");
    }
  } else {
    EPSolarTracerInputRegister reg;
    if (epsolar.get_register(0x9010, 1, reg)) {
      Serial.print("0x9010="); Serial.println(reg.getIntValue(0));
    }
  }
  return true;
}

bool eqcycle(const LineParser& lineparser)
{
  if (lineparser.get_count() > 1) {
    if (!isdigit(lineparser[1][0])) {
      Serial.println("Invalid parameter. (must be a number)");
      return true;
    }

    uint16_t val = (uint16_t)atoi(lineparser[1]);

    if (epsolar.put_register(0x9016, val)) {
      Serial.print("Equalization charging cycle set to ");
      Serial.print(val);
      Serial.println(" days.");
    } else {
      Serial.println("Failed");
    }
  } else {
    EPSolarTracerInputRegister reg;
    if (epsolar.get_register(0x9016, 1, reg)) {
      Serial.print("Equalization charging cycle: ");
      Serial.print(reg.getIntValue(0));
      Serial.println(" days.");
    }
  }
  return true;
}

bool ratedvoltagecode(const LineParser& lineparser)
{
  static const char* str[] = { "Auto", "12V", "24V" };
  if (lineparser.get_count() > 1) {
    if (!isdigit(lineparser[1][0])) {
      Serial.println("Invalid parameter. (must be a number)");
      return true;
    }

    uint16_t val = (uint16_t)atoi(lineparser[1]);
    if (val > 2) {
      Serial.println("must be 0(Auto), 1(12V) or 2(24V)");
      return true;
    }

    if (epsolar.put_register(0x9067, val)) {
      Serial.print("Battery rated voltage code set to ");
      Serial.print(val);
      Serial.print('(');
      Serial.print(str[val]);
      Serial.println(')');
    } else {
      Serial.println("Failed");
    }
  } else {
    EPSolarTracerInputRegister reg;
    if (epsolar.get_register(0x9067, 1, reg)) {
      int val = reg.getIntValue(0);
      Serial.print("Battery rated voltage code: ");
      Serial.print(val);
      Serial.print('(');
      Serial.print(str[val]);
      Serial.println(')');
    }
  }
  return true;
}

bool ls(const LineParser& lineparser)
{
  SPIFFS.begin();
#if defined(ARDUINO_ARCH_ESP8266)
  Dir dir = SPIFFS.openDir("/");
  while (dir.next()) {
    Serial.print(dir.fileName());
    File f = dir.openFile("r");
    Serial.println(String(" ") + f.size());
    f.close();
  }
#elif defined(ARDUINO_ARCH_ESP32)
  File dir = SPIFFS.open("/");
  File entry;
  while ((bool)(entry = dir.openNextFile())) {
    Serial.print(entry.name());
    Serial.println(String(" ") + entry.size());
    entry.close();
  }
#endif
  SPIFFS.end();
  return true;
}

bool partitions(const LineParser& lineparser)
{
#ifdef ARDUINO_ARCH_ESP32
  size_t ul;
  esp_partition_iterator_t _mypartiterator;
  const esp_partition_t *_mypart;
  ul = spi_flash_get_chip_size(); Serial.print("Flash chip size: "); Serial.println(ul);
  Serial.println("Partiton table:");
  _mypartiterator = esp_partition_find(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_ANY, NULL);
  if (_mypartiterator) {
    do {
      _mypart = esp_partition_get(_mypartiterator);
      printf("%x - %x - %x - %x - %s - %i\r\n", _mypart->type, _mypart->subtype, _mypart->address, _mypart->size, _mypart->label, _mypart->encrypted);
    } while (_mypartiterator = esp_partition_next(_mypartiterator));
  }
  esp_partition_iterator_release(_mypartiterator);
  _mypartiterator = esp_partition_find(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, NULL);
  if (_mypartiterator) {
    do {
      _mypart = esp_partition_get(_mypartiterator);
      printf("%x - %x - %x - %x - %s - %i\r\n", _mypart->type, _mypart->subtype, _mypart->address, _mypart->size, _mypart->label, _mypart->encrypted);
    } while (_mypartiterator = esp_partition_next(_mypartiterator));
  }
  esp_partition_iterator_release(_mypartiterator);
#else
  Serial.println("Not implemented in this platform");
#endif
  return true;
}

bool rssi(const LineParser& lineparser)
{
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected.");
    return true;
  }
  // else
  Serial.print("RSSI:");
  Serial.println(WiFi.RSSI());
  return true;
}

bool process_command_line(const char* line) // true = go to next line,  false = go to next loop
{
  LineParser lineparser(line);
  if (lineparser.get_count() == 0 || lineparser[0][0] =='#') return true;

  static struct {
    const char* cmd;
    bool (*func)(const LineParser&);
  } command_table[] = {
    { "exit", _exit }, { "quit", _exit },
    { "defaultmode", defaultmode },
    { "nodename", nodename },
    { "ssid", ssid },
    { "key", key },
    { "servername", servername },
    { "port", port },
    { "save", save },
    { "pw", pw },
    { "pw1", pwX },
    { "pw2", pwX },
    { "debug", debug },
    { "deviceinfo", device_info },
    { "uptime", uptime },
    { "reboot", reboot },
    { "read9022", read9022 },
    { "r9010", r9010 },
    { "eqcycle", eqcycle },
    { "ratedvoltagecode", ratedvoltagecode },
    { "ls", ls },
    { "partitions", partitions},
    { "rssi", rssi },
    { NULL, NULL }
  };

  for (int i = 0; command_table[i].cmd; i++) {
    if (strcmp(lineparser[0], command_table[i].cmd) == 0) return command_table[i].func(lineparser);
  }

  if (strcmp(lineparser[0], "?") == 0 || strcmp(lineparser[0], "help") == 0) {
    Serial.print("Available commands: ");
    for (int i = 0; command_table[i].cmd; i++) {
      Serial.print(command_table[i].cmd);
      Serial.print(" ");
    }
    Serial.println();
  } else {
    Serial.println("Unrecognized command.");
  }
  return true;
}
