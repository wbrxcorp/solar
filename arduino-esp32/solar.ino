// arduino --upload --board espressif:esp32:mhetesp32minikit:FlashFreq=80,UploadSpeed=921600 --port /dev/ttyUSB0 solar.ino
// arduino --upload --board esp8266com:esp8266:d1_mini:CpuFrequency=80,FlashSize=4M1M,UploadSpeed=115200 --port /dev/ttyUSB0 solar.ino
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

#define HIBYTE(word) ((uint8_t)((word & 0xff00) >> 8))
#define LOBYTE(word) ((uint8_t)(word & 0xff))

#ifdef ARDUINO_ARCH_ESP32
  // pin definitions for MH-ET ESP32 Mini Kit
  #define RS485_TX_SOCKET 17
  #define RS485_RX_SOCKET 16
  #define RS485_RTS_SOCKET 21
  #define COMMAND_LINE_ONLY_MODE_SOCKET 22

  // For Non-SPI mode
  #define PW1_SW_SOCKET 18
  #define PW1_LED_SOCKET 19
  #define PW2_SW_SOCKET 23
  #define PW2_LED_SOCKET 5

  // For SPI mode
  #define ALT_PW1_SW_SOCKET 35
  #define ALT_PW1_LED_SOCKET 33
  #define ALT_PW2_SW_SOCKET 34
  #define ALT_PW2_LED_SOCKET 14 // TMS at default
#elif ARDUINO_ARCH_ESP8266
  // pin definitions for D1 Mini
  #define RS485_TX_SOCKET D3
  #define RS485_RX_SOCKET D4
  #define RS485_RTS_SOCKET D2
  #define COMMAND_LINE_ONLY_MODE_SOCKET D1

  #define PW1_SW_SOCKET D5
  #define PW1_LED_SOCKET D6
  #define PW2_SW_SOCKET D7
  #define PW2_LED_SOCKET D8
#endif

#define OPERATION_MODE_NORMAL 0
#define OPERATION_MODE_COMMAND_LINE 1
#define OPERATION_MODE_COMMAND_LINE_ONLY 2

#define REPORT_INTERVAL 5000
#define ACPI_SHUTDOWN_TIMEOUT 10000
#define FORCE_SHUTDOWN_TIMEOUT 6000
#define MESSAGE_TIMEOUT 10000

const char* DEFAULT_NODENAME = "kennel01";
const char* DEFAULT_SERVERNAME = "_solar._tcp";
const uint16_t DEFAULT_PORT = 29574; // default server port number

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

class EPSolarTracerDeviceInfo {
  char* vendor_name;
  char* product_code;
  char* revision;
public:
  EPSolarTracerDeviceInfo() : vendor_name(NULL), product_code(NULL), revision(NULL) {
    ;
  }
  ~EPSolarTracerDeviceInfo() {
    if (vendor_name) delete [] vendor_name;
    if (product_code) delete [] product_code;
    if (revision) delete[] revision;
  }
  void set_value(uint8_t object_id, uint8_t object_length, const uint8_t* object_value) {
    if (object_id == 0x00/*VendorName*/) {
      if (vendor_name) delete[] vendor_name;
      vendor_name = new char[object_length + 1];
      memcpy(vendor_name, object_value, object_length);
      vendor_name[object_length] = '\0';
    } else if (object_id == 0x01/*ProductCode*/) {
      if (product_code) delete[] product_code;
      product_code = new char[object_length + 1];
      memcpy(product_code, object_value, object_length);
      product_code[object_length] = '\0';
    } else if (object_id == 0x02/*MajorMinorRevision*/) {
      if (revision) delete[] revision;
      revision = new char[object_length + 1];
      memcpy(revision, object_value, object_length);
      revision[object_length] = '\0';
    };
  }
  const char* get_vendor_name() const { return vendor_name; }
  const char* get_product_code() const { return product_code; }
  const char* get_revision() const { return revision; }
};

class EPSolarTracerInputRegister {
  uint8_t* _data;
  size_t _size;

  inline uint16_t MKWORD(uint8_t hi, uint8_t lo) const {
    return ((uint16_t)hi) << 8 | lo;
  }
  inline uint32_t MKDWORD(uint16_t hi, uint16_t lo) const {
    return ((uint32_t)hi) << 16 | lo;
  }
public:
  EPSolarTracerInputRegister() : _data(NULL), _size(0) {
  }
  EPSolarTracerInputRegister(const uint8_t* data, size_t size) {
    setData(data, size);
  }
  EPSolarTracerInputRegister(const EPSolarTracerInputRegister& other) {
    setData(other.data(), other.size());
  }
  ~EPSolarTracerInputRegister() {
    if (_data) delete []_data;
  }
  void setData(const uint8_t* data, size_t size) {
    if (_data && size > _size) {
      delete []_data;
      _data = NULL;
    }
    if (_data == NULL) {
      _data = new uint8_t[size];
    }
    memcpy(_data, data, size);
    _size = size;
  }
  size_t size() const { return _size; }
  const uint8_t* data() const { return _data; }

  int getIntValue(size_t offset) const {
    if (offset > _size - 2) return 0;
    return (int)MKWORD(_data[offset], _data[offset + 1]);
  }

  float getFloatValue(size_t offset) const {
    if (offset > _size - 2) return 0.0f;
    return (float)MKWORD(_data[offset], _data[offset + 1]) / 100.0f;
  }

  double getDoubleValue(size_t offset) const {
    if (offset > _size - 4) return 0.0;
    return (double)MKDWORD(MKWORD(_data[offset + 2], _data[offset + 3]), MKWORD(_data[offset], _data[offset + 1])) / 100.0;
  }

  uint64_t getRTCValue(size_t offset) const {
    if (offset > _size - 6) return 0L;
    uint16_t r9013 = MKWORD(_data[offset], _data[offset + 1]);
    uint16_t r9014 = MKWORD(_data[offset + 2], _data[offset + 3]);
    uint16_t r9015 = MKWORD(_data[offset + 4], _data[offset + 5]);
    uint16_t year = 2000 + (r9015 >> 8);
    uint8_t month = r9015 & 0x00ff;
    uint8_t day = r9014 >> 8;
    uint8_t hour = r9014 & 0x00ff;
    uint8_t minute = r9013 >> 8;
    uint8_t second = r9013 & 0x00ff;
    return year * 10000000000LL + month * 100000000LL + day * 1000000L
      + hour * 10000L + minute * 100 + second;
  }

  bool getBoolValue(uint16_t bit_index) const { // for coils
    uint8_t byte_index = bit_index / 8;
    if (byte_index >= _size) return false;
    bit_index = bit_index % 8;
    return (_data[byte_index] >> bit_index) > 0;
  }
};

LineBuffer receive_buffer;
LineBuffer cmdline_buffer('\r'/*cu sends \r instead of \n*/);

#ifdef ARDUINO_ARCH_ESP32
  HardwareSerial RS485(1);  // Use UART1 (need to change TX/RX pins)
#elif ARDUINO_ARCH_ESP8266
  SoftwareSerial RS485(RS485_RX_SOCKET, RS485_TX_SOCKET, false, 256);
#endif
WiFiClient tcp_client;

uint8_t operation_mode = OPERATION_MODE_NORMAL;
unsigned long last_report_time = 0;
char session_id[48] = "";
uint8_t battery_rated_voltage = 0; // 12 or 24(V)
uint8_t temperature_compensation_coefficient = 0; // 0-9(mV)
#ifdef ARDUINO_ARCH_ESP8266
  bool mdns_started = false;
#endif

struct {
  char nodename[32];
  char ssid[34];
  char key[64];
  char servername[48];
  uint16_t port;
  uint16_t crc;
} config;

struct {
  int pw1_sw = PW1_SW_SOCKET;
  int pw1_led = PW1_LED_SOCKET;
  int pw2_sw = PW2_SW_SOCKET;
  int pw2_led = PW2_LED_SOCKET;
} edogawa_unit_pins;

uint16_t update_crc(uint16_t crc, uint8_t val)
{
  crc ^= (uint16_t)val;
  for (int i = 8; i != 0; i--) {
    if ((crc & 0x0001) != 0) {
      crc >>= 1;
      crc ^= 0xA001;
    } else {
      crc >>= 1;
    }
  }
  return crc;
}

void put_crc(uint8_t* message, size_t payload_size)
{
  uint16_t crc = 0xFFFF;
  for (int pos = 0; pos < payload_size; pos++) {
    crc = update_crc(crc, message[pos]);
  }
  message[payload_size] = LOBYTE(crc);
  message[payload_size + 1] = HIBYTE(crc);
}

void send_modbus_message(const uint8_t* message, size_t size)
{
  while(RS485.available()) RS485.read();
  digitalWrite(RS485_RTS_SOCKET,HIGH);
  //delayMicroseconds(500);
  RS485.write(message, size);
  RS485.flush();
  delayMicroseconds(500);
  digitalWrite(RS485_RTS_SOCKET,LOW);
}

// http://www.modbus.org/docs/Modbus_Application_Protocol_V1_1b.pdf
bool get_device_info(EPSolarTracerDeviceInfo& info, int max_retry = 5)
{
  byte message[] = {0x01, 0x2b, 0x0e, 0x01/*basic info*/,0x00, 0x00, 0x00 };
  put_crc(message, sizeof(message) - 2);

  send_modbus_message(message, sizeof(message));

  byte hdr[8];
  int retry_count = 0;
  while (retry_count < max_retry) {
    if (RS485.readBytes(hdr, sizeof(hdr)) && memcmp(hdr, message, 4) == 0) {
      uint16_t crc = 0xffff;
      for (int i = 0; i < sizeof(hdr); i++) crc = update_crc(crc, hdr[i]);

      uint8_t num_objects = hdr[7];
      for (int i = 0; i < num_objects; i++) {
        uint8_t object_hdr[2];
        if (RS485.readBytes(object_hdr, sizeof(object_hdr)) != sizeof(object_hdr)) break;
        crc = update_crc(update_crc(crc, object_hdr[0]), object_hdr[1]);
        uint8_t object_value[object_hdr[1]];
        if (RS485.readBytes(object_value, sizeof(object_value)) != sizeof(object_value)) break;
        for (int i = 0; i < sizeof(object_value); i++) crc = update_crc(crc, object_value[i]);
        info.set_value(object_hdr[0], object_hdr[1], object_value);
      }
      // crc check
      uint8_t rx_crc[2] = {0, 0};
      if (RS485.readBytes(rx_crc, sizeof(rx_crc)) == sizeof(rx_crc) && !RS485.available() && rx_crc[0] == LOBYTE(crc) && rx_crc[1] == HIBYTE(crc)) return true;
    }
    // else
    while (RS485.available()) RS485.read(); // discard remaining bytes
    Serial.println("Retrying...");
    delay(200);
    retry_count++;
  }
}

void print_bytes(const uint8_t* bytes, size_t size)
{
  for (int i = 0; i < size; i++) {
    char buf[8];
    sprintf(buf, "%02x ", bytes[i]);
    Serial.print(buf);
  }
  Serial.println();
}

bool get_register(uint16_t addr, uint8_t num, EPSolarTracerInputRegister& reg, int max_retry = 10)
{
  uint8_t function_code = 0x04; // Read Input Register
  if (addr >= 0x9000 && addr < 0x9100) function_code = 0x03; // Read Holding Register
  if (addr < 0x2000) function_code = 0x01; // Read Coil Status

  uint8_t message[] = {0x01, function_code, HIBYTE(addr), LOBYTE(addr), 0x00, num, 0x00, 0x00 };
  put_crc(message, sizeof(message) - 2);
  //print_bytes(message, sizeof(message));

  for (int i = 0; i < max_retry; i++) {
    send_modbus_message(message, sizeof(message));

    uint8_t hdr[3];
    if (RS485.readBytes(hdr, sizeof(hdr)) == sizeof(hdr)) {
      //Serial.print("hdr received: ");
      //print_bytes(hdr, 3);
      if (hdr[0] == message[0] && hdr[1] == message[1]) { // check function code and slave address
        size_t data_size = (size_t)hdr[2];
        if (data_size < 128) { // too big data
          uint8_t buf[data_size];
          if (RS485.readBytes(buf, sizeof(buf)) == sizeof(buf)) {
            uint8_t rx_crc[2] = {0, 0};
            if (RS485.readBytes(rx_crc, sizeof(rx_crc)) == sizeof(rx_crc) && !RS485.available()) {
              // crc check
              uint16_t crc = update_crc(update_crc(update_crc(0xFFFF,hdr[0]), hdr[1]), hdr[2]);
              for (int i = 0; i < sizeof(buf); i++) crc = update_crc(crc, buf[i]);
              if (rx_crc[0] == LOBYTE(crc) && rx_crc[1] == HIBYTE(crc)) {
                reg.setData(buf, data_size);
                return true;
              } else {
                Serial.println("CRC doesn't match");
              }
            }
          }
        }
      }
    }
    // else
    int available = RS485.available();
    if (available) {
      uint8_t buf[available];
      int size = RS485.readBytes(buf, sizeof(buf));
      Serial.print("Remains: ");
      print_bytes(buf, size);
    }
    Serial.println("Retrying...");
    delay(200);
  }
  return false;
}

void connect()
{
  unsigned long retry_delay = 1;/*sec*/
  String _tmp_hostname = "nodexxx";
  _tmp_hostname += config.nodename;
  const char* tmp_hostname = _tmp_hostname.c_str();
  int servername_len = strlen(config.servername);

  while (true) {
    int connected = 0;

#ifdef ARDUINO_ARCH_ESP32
    if (servername_len > 6 && strcmp(config.servername + servername_len - 6, ".local") == 0) {
      Serial.printf("Querying hostname '%s' via mDNS...", config.servername);
      char servername_without_suffix[servername_len - 5];
      strncpy(servername_without_suffix, config.servername, servername_len - 6);
      servername_without_suffix[servername_len - 6] = '\0';

      if (!mdns_init()) {
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
      if (mdns_started || MDNS.begin(tmp_hostname)) {
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

bool put_register(uint16_t addr, uint16_t data)
{
  uint8_t function_code = 0x06; // Preset Single Register(06)
  if (addr < 0x2000) function_code = 0x05; // Force Single Coil
  byte message[] = {0x01, function_code, HIBYTE(addr), LOBYTE(addr), HIBYTE(data), LOBYTE(data), 0x00, 0x00 };
  put_crc(message, sizeof(message) - 2);

  send_modbus_message(message, sizeof(message));
  delay(50);
  while (RS485.available()) RS485.read(); // simply discard response(TODO: check the response)
  return true;
}

bool put_registers(uint16_t addr, uint16_t* data, uint16_t num)
{
  uint8_t data_size_in_bytes = (uint8_t)(sizeof(*data) * num);
  size_t message_size = 9/*slave address, func code, start addr(H+L), num(H+L), length in bytes, ... , crc(L/H)*/ + data_size_in_bytes;
  byte message[message_size];
  message[0] = 0x01;
  message[1] = 0x10;
  message[2] = HIBYTE(addr);
  message[3] = LOBYTE(addr);
  message[4] = HIBYTE(num);
  message[5] = LOBYTE(num);
  message[6] = data_size_in_bytes;
  for (int i = 0; i < num; i++) {
    message[7 + i * 2] = HIBYTE(data[i]);
    message[8 + i * 2] = LOBYTE(data[i]);
  }
  put_crc(message, message_size - 2);

  //print_bytes(message, sizeof(message));
  send_modbus_message(message, sizeof(message));
  delay(50);
  while (RS485.available()) RS485.read(); // simply discard response(TODO: check the response)
  return true;
}

void poweron_pw()
{
  put_register(0x0002/*manual load control*/, (uint16_t)0xff00);
}

void poweroff_pw()
{
  put_register(0x0002/*manual load control*/, (uint16_t)0x0000);
}

bool read_pw1()
{
  if (edogawa_unit_pins.pw1_led < 0) return false;
  // else
  return digitalRead(edogawa_unit_pins.pw1_led) == LOW;
}

void poweron_pw1()
{
  if (edogawa_unit_pins.pw1_sw < 0) {
    Serial.println("pw1 not available");
    return;
  }
  // else

  digitalWrite(edogawa_unit_pins.pw1_sw, LOW);
  poweron_pw(); // main power on
  delay(500);
  digitalWrite(edogawa_unit_pins.pw1_sw, HIGH);
  delay(200);
  digitalWrite(edogawa_unit_pins.pw1_sw, LOW);
}

void poweroff_pw1()
{
  if (edogawa_unit_pins.pw1_sw < 0) {
    Serial.println("pw1 not available");
    return;
  }
  // else

  digitalWrite(edogawa_unit_pins.pw1_sw, LOW);
  delay(100);
  digitalWrite(edogawa_unit_pins.pw1_sw, HIGH);
  delay(200);
  digitalWrite(edogawa_unit_pins.pw1_sw, LOW);
  // Wait for ACPI shutdown
  unsigned long time = millis();
  while (read_pw1()) {
    if (millis() - time > ACPI_SHUTDOWN_TIMEOUT) {
      // force OFF
      time = millis();
      digitalWrite(edogawa_unit_pins.pw1_sw, HIGH);
      while (read_pw1()) {
        if (millis() - time > FORCE_SHUTDOWN_TIMEOUT) break;
        // else
        delay(100);
      }
      digitalWrite(edogawa_unit_pins.pw1_sw, LOW);
      break;
    }
    delay(100);
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
      put_register(0x9000/*Battery type*/, (uint16_t)battery_type);
      Serial.print("Battery type saved: ");
      Serial.println(battery_type);
    } else if (strcmp(key, "bc") == 0) {
      int battery_capacity= atoi(value);
      put_register(0x9001/*Battery capacity*/, (uint16_t)battery_capacity);
      Serial.print("Battery capacity saved: ");
      Serial.print(battery_capacity);
      Serial.println("Ah");
    } else if (strcmp(key, "pw") == 0 && isdigit(value[0])) { // main power
      int pw = atoi(value);
      if (pw == 0) {
        Serial.println("Power OFF");
        if (read_pw1()) poweroff_pw1(); // atx power off
        poweroff_pw();
      } else if (pw == 1) {
        Serial.println("Power ON");
        poweron_pw();
      }
    } else if (strcmp(key, "pw1") == 0 && (isdigit(value[0]) || value[0] == '-')) { // atx power
      int pw1 = atoi(value);
      bool pw1_on = read_pw1();
      if (pw1_on && pw1 == 0) { // power off
        Serial.println("Power1 OFF");
        poweroff_pw1();
      } else if (!pw1_on && pw1 == 1) { // power on
        Serial.println("Power1 ON");
        poweron_pw1(); // main power on
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
      put_registers(0x9013/*Real Time Clock*/, data, 3);
      char buf[32];
      sprintf(buf, "Date saved: 20%02u-%02u-%02u %02u:%02u:%02u", year, month, day, hour, minute, second);
      Serial.println(buf);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(RS485_RTS_SOCKET, OUTPUT);
  pinMode(COMMAND_LINE_ONLY_MODE_SOCKET, INPUT_PULLUP); // Short to enter command line only mode
  pinMode(LED_BUILTIN, OUTPUT);

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


  if (false/*use_spi*/) {
#ifdef ALT_PW1_SW_SOCKET
    edogawa_unit_pins.pw1_sw = ALT_PW1_SW_SOCKET;
#else
    edogawa_unit_pins.pw1_sw = -1;
#endif
#ifdef ALT_PW1_LED_SOCKET
    edogawa_unit_pins.pw1_led = ALT_PW1_LED_SOCKET;
#else
    edogawa_unit_pins.pw1_led = -1;
#endif
#ifdef ALT_PW2_SW_SOCKET
    edogawa_unit_pins.pw2_sw = ALT_PW2_SW_SOCKET;
#else
    edogawa_unit_pins.pw2_sw = -1;
#endif
#ifdef ALT_PW2_LED_SOCKET
    edogawa_unit_pins.pw2_led = ALT_PW2_LED_SOCKET;
#else
    edogawa_unit_pins.pw2_led = -1;
#endif
  }

  if (edogawa_unit_pins.pw1_sw >= 0) pinMode(edogawa_unit_pins.pw1_sw, OUTPUT);
  if (edogawa_unit_pins.pw1_led >= 0) pinMode(edogawa_unit_pins.pw1_led, INPUT_PULLUP);
  if (edogawa_unit_pins.pw2_sw >= 0) pinMode(edogawa_unit_pins.pw2_sw, OUTPUT);
  if (edogawa_unit_pins.pw2_led >= 0) pinMode(edogawa_unit_pins.pw2_led, INPUT_PULLUP);

  if (digitalRead(COMMAND_LINE_ONLY_MODE_SOCKET) == LOW) { // LOW == SHORT(pulled up)
    Serial.print("Entering command line only mode...\r\n# ");
    operation_mode = OPERATION_MODE_COMMAND_LINE_ONLY;
    return;
  }

#ifdef ARDUINO_ARCH_ESP32
  RS485.begin(115200, SERIAL_8N1, RS485_RX_SOCKET, RS485_TX_SOCKET); // USE 16/17 pins originally assigned to UART2
#elif ARDUINO_ARCH_ESP8266
  RS485.begin(115200);
#endif
  EPSolarTracerDeviceInfo info;
  if (get_device_info(info)) {
    Serial.print("Vendor: ");
    Serial.println(info.get_vendor_name());
    Serial.print("Product: ");
    Serial.println(info.get_product_code());
    Serial.print("Revision: ");
    Serial.println(info.get_revision());
  } else {
    Serial.println("Getting charge controller device info failed!");
  }

  EPSolarTracerInputRegister reg;
  if (get_register(0x9013/*Real Time Clock*/, 3, reg, 3)) {
    uint64_t rtc = reg.getRTCValue(0);
    char buf[32];
    sprintf(buf, "RTC: %lu %06lu", (uint32_t)(rtc / 1000000L), (uint32_t)(rtc % 1000000LL));
    Serial.println(buf);
    if (get_register(0x9000/*battery type, battery capacity*/, 2, reg)) {
      const char* battery_type_str[] = { "User Defined", "Sealed", "GEL", "Flooded" };
      int battery_type = reg.getIntValue(0);
      int battery_capacity = reg.getIntValue(2);
      sprintf(buf, "Battery type: %d(", battery_type);
      strcat(buf, battery_type_str[battery_type]);
      sprintf(buf + strlen(buf), "), %dAh", battery_capacity);
      Serial.println(buf);
      if (get_register(0x311d/*Battery real rated voltage*/, 1, reg)) {
        battery_rated_voltage = (uint8_t)reg.getFloatValue(0);
        Serial.print("Battery real rated voltage: ");
        Serial.print((int)battery_rated_voltage);
        Serial.println("V");
        if (get_register(0x9002/*Temperature compensation coefficient*/, 1, reg)) {
          temperature_compensation_coefficient = (uint8_t)reg.getFloatValue(0);
          Serial.print("Temperature compensation coefficient: ");
          Serial.print((int)temperature_compensation_coefficient);
          Serial.println("mV/Cecelsius degree/2V");
          if (get_register(0x0006/*Force the load on/off*/, 1, reg)) {
            Serial.print("Force the load on/off: ");
            Serial.println(reg.getBoolValue(0)? "on" : "off(used for test)");
          }
        }
      }
    }
    if (put_register(0x903d/*Load controlling mode*/, (uint16_t)0)) {
      Serial.println("Load controlling mode set to 0(Manual)");
    }
    if (put_register(0x906a/*Default load on/off in manual mode*/, (uint16_t)1)) {
      Serial.println("Default load on/off in manual mode set to 1(on)");
    }
    if (put_register(0x0006/*Force the load on/off*/, (uint16_t)0xff00)) {
      Serial.println("Force the load on/off set to 'on'");
    }
  } else {
    Serial.println("Getting charge controller settings failed!");
  }

  Serial.print("Connecting to WiFi AP");
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

  connect();

#ifdef ARDUINO_ARCH_ESP32
  esp_wifi_set_ps(WIFI_PS_MODEM) == ESP_OK;
#elif ARDUINO_ARCH_ESP8266
  wifi_set_sleep_type(MODEM_SLEEP_T);
#endif
}

bool process_command_line(const char* line) // true = go to next line,  false = go to next loop
{
  LineParser lineparser(line);
  if (lineparser.get_count() == 0 || lineparser[0][0] =='#') return true;

  if (strcmp(lineparser[0], "exit") == 0 || strcmp(lineparser[0], "quit") == 0) {
    if (operation_mode == OPERATION_MODE_COMMAND_LINE_ONLY) {
      Serial.println("This is 'command line only' mode. make sure wifi config is right and restart system.");
      return true;
    }
    // else
    operation_mode = OPERATION_MODE_NORMAL;
    Serial.println("Exitting command line mode.");
    return false;
  } else if (strcmp(lineparser[0], "nodename") == 0) {
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
  } else if (strcmp(lineparser[0], "ssid") == 0) {
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
  } else if (strcmp(lineparser[0], "key") == 0) {
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
  } else if (strcmp(lineparser[0], "servername") == 0) {
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
  } else if (strcmp(lineparser[0], "port") == 0) {
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
  } else if (strcmp(lineparser[0], "save") == 0) {
    uint8_t* p = (uint8_t*)&config;
    config.crc = 0xffff;
    for (size_t i = 0; i < sizeof(config) - sizeof(config.crc); i++) {
      config.crc = update_crc(config.crc, p[i]);
    }
    Serial.print("Writing config to EEPROM...");
    EEPROM.begin(sizeof(config));
    EEPROM.put(0, config);
    EEPROM.commit();
    EEPROM.end();

    Serial.println("Done.");
  } else if (strcmp(lineparser[0], "?") == 0 || strcmp(lineparser[0], "help") == 0) {
    Serial.println("Available commands: nodename ssid key servername port exit");
  } else {
    Serial.println("Unrecognized command.");
  }
  return true;
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
  // enter command line mode when enter presses
  if (Serial.available() && Serial.read() == '\r') {
    operation_mode = OPERATION_MODE_COMMAND_LINE;
    Serial.println("Entering command line mode. '?' to help, 'exit' to exit.");
    Serial.print("# ");
    return;
  }

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
    double lkwh;
    double kwh;
    int pw;

    bool success = false;

    // TODO: apply https://github.com/wbrxcorp/solar/commit/a664b0577e7c9bbcac6448741c43e58ce6750e43
    if (get_register(0x3100, 6, reg)) {
      piv = reg.getFloatValue(0);
      pia = reg.getFloatValue(2);
      piw = reg.getDoubleValue(4);
      bv = reg.getFloatValue(8);
      poa = reg.getFloatValue(10);
      delay(50);
      if (get_register(0x310e, 3, reg)) {
        load = reg.getDoubleValue(0);
        temp = reg.getFloatValue(4);
        delay(50);
        if (get_register(0x3304, 1, reg)) {
          lkwh = reg.getDoubleValue(0);
          delay(50);
          if (get_register(0x330c, 1, reg)) {
            kwh = reg.getDoubleValue(0);
            if (get_register(0x0002, 1, reg)) { // Manual control the load
              pw = reg.getBoolValue(0)? 1 : 0;
              success = true;
            }
          }
        }
      }
    }

    while (true) {
      char buf[64];
      if (success) {
        strcpy(buf, "DATA\tpiv:");
        dtostrf(piv, 4, 2, buf + strlen(buf));
        tcp_client.write(buf);

        strcpy(buf, "\tpia:");
        dtostrf(pia, 4, 2, buf + strlen(buf));
        tcp_client.write(buf);

        strcpy(buf, "\tpiw:");
        dtostrf(piw, 4, 2, buf + strlen(buf));
        tcp_client.write(buf);

        strcpy(buf, "\tbv:");
        dtostrf(bv, 4, 2, buf + strlen(buf));
        tcp_client.write(buf);

        strcpy(buf, "\tpoa:");
        dtostrf(poa, 4, 2, buf + strlen(buf));
        tcp_client.write(buf);

        strcpy(buf, "\tload:");
        dtostrf(load, 4, 2, buf + strlen(buf));
        tcp_client.write(buf);

        strcpy(buf, "\ttemp:");
        dtostrf(temp, 4, 2, buf + strlen(buf));
        tcp_client.write(buf);

        strcpy(buf, "\tlkwh:");
        dtostrf(lkwh, 4, 2, buf + strlen(buf));
        tcp_client.write(buf);

        strcpy(buf, "\tkwh:");
        dtostrf(kwh, 4, 2, buf + strlen(buf));
        tcp_client.write(buf);

        /*
        sprintf(buf, "\tpw:%d", pw);
        sprintf(buf + strlen(buf), "\tpw1:%d", read_pw1()? 1 : 0);
        tcp_client.write(buf);
        */
      } else {
        tcp_client.write("NODATA");
      }

      if (session_id[0]) {
        strcpy(buf, "\tsession:");
        strcat(buf, session_id);
        tcp_client.write(buf);
      }

      strcpy(buf, "\tnodename:");
      strcat(buf, config.nodename);

      if (send_message(buf)) break;
      //else
      // Perform autoreconnect when something fails
      Serial.println("Connection error. performing autoreconnect...");
      delay(970);
      connect();
    }

    last_report_time = current_time;
  }
  digitalWrite(LED_BUILTIN, current_time / 1000 % 2 == 0);
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
