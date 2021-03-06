// arduino --upload --board arduino:avr:uno --port /dev/ttyACM0 solar.ino
// arduino --upload --board arduino:avr:mega:cpu=atmega2560 --port /dev/ttyACM0 solar.ino

#include <SoftwareSerial.h>
#include <EEPROM.h>
#include <util/crc16.h>

#define HIBYTE(word) ((uint8_t)((word & 0xff00) >> 8))
#define LOBYTE(word) ((uint8_t)(word & 0xff))

//#define DEBUG_AT_COMMANDS

#ifdef ARDUINO_AVR_UNO
  #define WIFI_RX_SOCKET 2
  #define WIFI_TX_SOCKET 3
  #define PW_SW_SOCKET 4
  #define PW_LED_SOCKET 5
  #define RS485_RX_SOCKET 6 // .. RO
  #define RS485_TX_SOCKET 7 // .. DI
  #define RS485_RTS_SOCKET 8 // .. DE-RE
  #define PW_IND_LED_SOCKET 9
  #define COMMAND_LINE_ONLY_MODE_SOCKET 10
#elif defined(ARDUINO_AVR_MEGA) || defined(ARDUINO_AVR_MEGA2560)
  #define USE_HARDWARE_SERIAL
  #define RS485_RTS_SOCKET 2 // .. DE-RE
  #define PW_SW_SOCKET 3
  #define PW_LED_SOCKET 4
  #define PW_IND_LED_SOCKET 5
  #define COMMAND_LINE_ONLY_MODE_SOCKET 7
#endif

#define REPORT_INTERVAL 5000
#define ACPI_SHUTDOWN_TIMEOUT 10000
#define FORCE_SHUTDOWN_TIMEOUT 6000
#define MESSAGE_TIMEOUT 10000

#define OPERATION_MODE_NORMAL 0
#define OPERATION_MODE_COMMAND_LINE 1
#define OPERATION_MODE_COMMAND_LINE_ONLY 2
#define OPERATION_MODE_WIFI_DIRECT 3

#define COMM_BUF_MAX 64   // set max line size to serial's internal buffer size

const uint16_t DEFAULT_PORT = 29574; // default server port number

#ifdef USE_HARDWARE_SERIAL
  #define WiFi Serial1 // RX=19,TX=18
  #define RS485 Serial2 // RX=17 .. RO, TX=16 .. DI
#else
  SoftwareSerial WiFi(WIFI_RX_SOCKET, WIFI_TX_SOCKET); // RX, TX
  SoftwareSerial RS485(RS485_RX_SOCKET, RS485_TX_SOCKET); // RX, TX
#endif

uint8_t operation_mode = OPERATION_MODE_NORMAL;
unsigned long last_report_time = 0;
char session_id[48];
uint8_t battery_rated_voltage = 0; // 12 or 24(V)
uint8_t temperature_compensation_coefficient = 0; // 0-9(mV)

struct {
  char nodename[32];
  char ssid[34];
  char key[64];
  char servername[48];
  uint16_t port;
  uint16_t crc;
} config;

class LineBuffer {
  char* buf = NULL;
  size_t max_size = 0;
  size_t size = 0;
  char terminator = '\n';
public:
  LineBuffer(size_t _max_size, char _terminator = '\n') : max_size(_max_size), terminator(_terminator) {
    this->buf = new char[_max_size];
  }
  ~LineBuffer() {
    delete [] buf;
  }
  bool push(char c) { // returns true if completed(terminated by \n)
    if (size < max_size - 1) buf[size++] = c;
    if (c == this->terminator && size == max_size) buf[max_size - 1] = this->terminator;
    return memchr(buf, this->terminator, size) != NULL;
  }
  bool push(const char* str) {
    for (const char* pt = str; *pt != '\0'; pt++) {
      if (size < max_size - 1) buf[size++] = *pt;
      if (*pt == this->terminator && size == max_size) buf[max_size - 1] = this->terminator;
    }
    return memchr(buf, this->terminator, size) != NULL;
  }
  bool push(const char* str, size_t len) {
    for (int i = 0; i < len; i++) {
      if (size < max_size - 1) buf[size++] = str[i];
      if (str[i] == this->terminator && size == max_size) buf[max_size - 1] = this->terminator;
    }
    return memchr(buf, this->terminator, size) != NULL;
  }
  int get_line_size() const {
    const char* pos = (const char*)memchr(buf, this->terminator, size);
    if (pos == NULL) return -1;
    return pos - buf;
  }
  bool pop(char* dst, size_t dstsize) {
    if (this->get_line_size() < 0) return false;
    const char* pt = this->buf;
    while (*pt != this->terminator) {
      if (pt - this->buf < dstsize - 1) *dst++ = *pt++;
    }
    *dst = '\0';

    pt++;
    size_t pos = (pt - this->buf);
    memmove(this->buf, pt, this->size - pos);
    this->size -= pos;

    return true;
  }
  void clear() { this->size = 0; }
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

LineBuffer receive_buffer(COMM_BUF_MAX);
LineBuffer cmdline_buffer(COMM_BUF_MAX, '\r'/*cu sends \r instead of \n*/);

class ExpectedCharSeq {
  const char* expect = NULL;
  char* buf = NULL;
  size_t len = 0;
public:
  ExpectedCharSeq(const char* _expect) : expect(_expect)
  {
    this->len = strlen(_expect);
    this->buf = new char[this->len];
    memset(this->buf, 0, this->len);
  }
  ~ExpectedCharSeq() { delete this->buf; }

  bool push(char c)
  {
    for (int i = 0; i < this->len - 1; i++) {
      this->buf[i] = this->buf[i + 1];
    }
    this->buf[this->len - 1] = c;
    return (*this);
  }

  operator bool() const { return memcmp(this->buf, this->expect, this->len) == 0; }
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

  uint16_t getWordValue(size_t offset) const {
    if (offset > _size - 2) return 0;
    return MKWORD(_data[offset], _data[offset + 1]);
  }

  int16_t getIntValue(size_t offset) const {
    return (int16_t)getWordValue(offset);
  }

  float getFloatValue(size_t offset) const {
    return (float)getIntValue(offset) / 100.0f;
  }

  uint32_t getDwordValue(size_t offset) const {
    if (offset > _size - 4) return 0.0;
    return MKDWORD(MKWORD(_data[offset + 2], _data[offset + 3]), MKWORD(_data[offset], _data[offset + 1]));
  }

  int32_t getLongValue(size_t offset) const {
    return (int32_t)getDwordValue(offset);
  }

  double getDoubleValue(size_t offset) const {
    return (double)getLongValue(offset) / 100.0;
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

#ifdef USE_HARDWARE_SERIAL
  // Hardware Serial doesn't need to listen explicitly
  void listen_rs485() { ; }
  void listen_wifi(){ ; }
#else
  void listen_rs485() { RS485.listen(); }
  void listen_wifi(){ WiFi.listen(); }
#endif

void send_modbus_message(const uint8_t* message, size_t size)
{
  digitalWrite(RS485_RTS_SOCKET,HIGH);
  delay(1);
  RS485.write(message, size);
#ifdef USE_HARDWARE_SERIAL
  RS485.flush();
#else
  delay(1);
#endif
  digitalWrite(RS485_RTS_SOCKET,LOW);
}

void wait_for(const char* expectedCharSeq)
{
  ExpectedCharSeq e(expectedCharSeq);
  while (true) {
    int available = WiFi.available();
    if (available > 0) {
      for (int i = 0; i < available; i++) {
        char c = WiFi.read();
#ifdef DEBUG_AT_COMMANDS
        Serial.write(c);
#endif
        if (e.push(c)) return;
      }
    } else delay(100);
  }
}

bool wait_for_result(const char* successCharSeq, const char* failCharSeq)
{
  ExpectedCharSeq success(successCharSeq);
  ExpectedCharSeq fail(failCharSeq);
  ExpectedCharSeq ready("\r\nready\r\n"); // for unexpected esp8266 reset
  while (true) {
    int available = WiFi.available();
    if (available > 0) {
      for (int i = 0; i < available; i++) {
        char c = WiFi.read();
#ifdef DEBUG_AT_COMMANDS
        Serial.write(c);
#endif
        if (success.push(c)) return true;
        if (fail.push(c) || ready.push(c)) return false;
      }
    } else delay(100);
  }
}

bool issue_at_command(const char* cmd, const char* failCharSeq = "\r\nERROR\r\n")
{
  WiFi.write(cmd);
  WiFi.write("\r\n");
  return wait_for_result("\r\nOK\r\n", failCharSeq);
}

void leave_transparent_transmission_mode()
{
  delay(30); // at least 20ms delay
  WiFi.write("+++");
  delay(30);
}

bool send_message(const char* message)
{
  WiFi.write(message, strlen(message));
  WiFi.write('\n');

  while (true) { // wait for response
    int available;
    unsigned long t = millis();
    while ((available = WiFi.available()) == 0) {
      delay(100);
      if (millis() > t + MESSAGE_TIMEOUT) {
        Serial.println("Message timeout.");
        leave_transparent_transmission_mode();
        return false;
      }
    }
    char buf[available];
    int r = WiFi.readBytes(buf, available);
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

void connect()
{
  unsigned long retry_delay = 1;/*sec*/
  char buf[128];
  while (true) {
    Serial.println(F("Connecting to WiFi AP..."));
    strcpy_P(buf, PSTR("AT+CWJAP_CUR=\""));
    strcat(buf, config.ssid);
    strcat_P(buf, PSTR("\",\""));
    strcat(buf, config.key);
    strcat_P(buf, PSTR("\""));
    if (issue_at_command(buf, "\r\nFAIL\r\n")) {
      Serial.println(F("Connecting to the server..."));
      sprintf_P(buf, PSTR("AT+CIPSTART=\"TCP\",\"%s\",%d,10"), config.servername, config.port);
      if (issue_at_command(buf)) {
        strcpy_P(buf, PSTR("AT+CIPMODE=1"));
        issue_at_command(buf);

      	WiFi.write("AT+CIPSEND\r\n");
      	if (wait_for_result("\r\n>", "\r\nERROR\r\n")) {
          session_id[0] = '\0'; // clear session id
      	  sprintf_P(buf, PSTR("INIT\tnodename:%s"), config.nodename);
      	  if (send_message(buf)) {
      	    Serial.println(F("Connection established."));
      	    return;
      	  }
      	}
      } else {
        // "ERROR"
        wait_for("CLOSED\r\n");
        Serial.println(F("Connecting to server failed."));
      }
    } else {
      Serial.println(F("Connecting to WiFi AP failed."));
    }
    // retry
    Serial.print(F("Performing retry ("));
    Serial.print(retry_delay);
    Serial.println(F("sec)..."));
    delay(retry_delay * 1000);
    retry_delay *= 2;
    if (retry_delay > 60) retry_delay = 60;
  }
}

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

  byte message[] = {0x01, function_code, HIBYTE(addr), LOBYTE(addr), 0x00, num, 0x00, 0x00 };
  put_crc(message, sizeof(message) - 2);
  //print_bytes(message, sizeof(message));

  for (int i = 0; i < max_retry; i++) {
    send_modbus_message(message, sizeof(message));
    uint8_t hdr[3];
    if (RS485.readBytes(hdr, sizeof(hdr)) == sizeof(hdr)) {
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
    while (RS485.available()) RS485.read(); // discard remaining bytes
    Serial.println("Retrying...");
    delay(200);
  }
  return false;
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

  send_modbus_message(message, sizeof(message));
  delay(50);
  while (RS485.available()) RS485.read(); // simply discard response(TODO: check the response)
  return true;
}

void setup() {
  pinMode(PW_SW_SOCKET, OUTPUT); // PC PWR BTN
  pinMode(PW_LED_SOCKET, INPUT_PULLUP); // PC PWR LED
  pinMode(COMMAND_LINE_ONLY_MODE_SOCKET, INPUT_PULLUP); // Short to enter command line only mode

  Serial.begin(9600);
  RS485.begin(115200);
  pinMode(RS485_RTS_SOCKET, OUTPUT);

#ifdef USE_HARDWARE_SERIAL
  WiFi.begin(115200); // Look, this is power of hardware serial.
#else
  //WiFi.begin(115200);
  //WiFi.write("AT+UART_DEF=9600,8,1,0,0\r\n");
  WiFi.begin(9600);
#endif

  // read config from EEPROM
  Serial.write("Loading config from EEPROM...");
  EEPROM.get(0, config);

  uint8_t* p = (uint8_t*)&config;
  uint16_t crc = 0;
  for (size_t i = 0; i < sizeof(config) - sizeof(config.crc); i++) {
    crc = _crc16_update(crc, p[i]);
  }
  if (crc != config.crc) {
    Serial.print(F("Checksum mismatch(expected="));
    Serial.print(crc);
    Serial.print(F(", actual="));
    Serial.print(config.crc);
    Serial.print(F("). entering command line only mode.\r\n# "));
    memset(&config, 0, sizeof(config));

    strcpy_P(config.nodename, PSTR("kennel01"));
    strcpy_P(config.ssid, PSTR("YOUR_ESSID"));
    strcpy_P(config.key, PSTR("YOUR_WPA_KEY"));
    strcpy_P(config.servername, PSTR("your.server"));
    config.port = DEFAULT_PORT;
    operation_mode = OPERATION_MODE_COMMAND_LINE_ONLY;
    return;
  }
  Serial.println(F("Done."));
  Serial.print(F("Nodename: "));
  Serial.println(config.nodename);
  Serial.print(("WiFi SSID: "));
  Serial.println(config.ssid);
  Serial.println(F("WiFi Password: *"));
  Serial.print(F("Server name: "));
  Serial.println(config.servername);
  Serial.print(F("Server port: "));
  Serial.println(config.port);

  if (digitalRead(COMMAND_LINE_ONLY_MODE_SOCKET) == LOW) { // LOW == SHORT(pulled up)
    Serial.print("Entering command line only mode...\r\n# ");
    operation_mode = OPERATION_MODE_COMMAND_LINE_ONLY;
    return;
  }

  // get info from caharge controller
  listen_rs485();
  EPSolarTracerDeviceInfo info;
  if (get_device_info(info), 3) {
    Serial.print("Vendor: ");
    Serial.println(info.get_vendor_name());
    Serial.print("Product: ");
    Serial.println(info.get_product_code());
    Serial.print("Revision: ");
    Serial.println(info.get_revision());
  } else {
    Serial.println(F("Getting charge controller device info failed!"));
  }

  EPSolarTracerInputRegister reg;
  if (get_register(0x9013/*Real Time Clock*/, 3, reg, 3)) {
    uint64_t rtc = reg.getRTCValue(0);
    char buf[32];
    sprintf_P(buf, PSTR("RTC: %lu %06lu"), (uint32_t)(rtc / 1000000L), (uint32_t)(rtc % 1000000LL));
    Serial.println(buf);
    if (get_register(0x9000/*battery type, battery capacity*/, 2, reg)) {
      const char* battery_type_str[] = { PSTR("User Defined"), PSTR("Sealed"), PSTR("GEL"), PSTR("Flooded") };
      int battery_type = reg.getIntValue(0);
      int battery_capacity = reg.getIntValue(2);
      sprintf_P(buf, PSTR("Battery type: %d("), battery_type);
      strcat_P(buf, battery_type_str[battery_type]);
      sprintf_P(buf + strlen(buf), PSTR("), %dAh"), battery_capacity);
      Serial.println(buf);
      if (get_register(0x311d/*Battery real rated voltage*/, 1, reg)) {
        battery_rated_voltage = (uint8_t)reg.getFloatValue(0);
        Serial.print(F("Battery real rated voltage: "));
        Serial.print((int)battery_rated_voltage);
        Serial.println(F("V"));
        if (get_register(0x9002/*Temperature compensation coefficient*/, 1, reg)) {
          temperature_compensation_coefficient = (uint8_t)reg.getFloatValue(0);
          Serial.print(F("Temperature compensation coefficient: "));
          Serial.print((int)temperature_compensation_coefficient);
          Serial.println(F("mV/Cecelsius degree/2V"));
          if (get_register(0x0006/*Force the load on/off*/, 1, reg)) {
            Serial.print("Force the load on/off: ");
            Serial.println(reg.getBoolValue(0)? F("on") : F("off(used for test)"));
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
    Serial.println(F("Getting charge controller settings failed!"));
  }

  listen_wifi();

  leave_transparent_transmission_mode();
  delay(970);
  issue_at_command("AT");
  issue_at_command("AT+CWQAP"); // disconnect from AP (error if not connected but no harm)
  issue_at_command("AT+CWMODE_CUR=1");
  connect();
}

void poweron_pw()
{
  listen_rs485();
  put_register(0x0002/*manual load control*/, (uint16_t)0xff00);
  listen_wifi();
}

void poweroff_pw()
{
  listen_rs485();
  put_register(0x0002/*manual load control*/, (uint16_t)0x0000);
  listen_wifi();
}

bool read_pw1()
{
  bool pw1_on = digitalRead(PW_LED_SOCKET) == LOW;
  digitalWrite(PW_IND_LED_SOCKET, pw1_on? HIGH : LOW);
  return pw1_on;
}

void poweron_pw1()
{
  digitalWrite(PW_SW_SOCKET, LOW);
  poweron_pw(); // main power on
  delay(500);
  digitalWrite(PW_SW_SOCKET, HIGH);
  delay(200);
  digitalWrite(PW_SW_SOCKET, LOW);
}

void poweroff_pw1()
{
  digitalWrite(PW_SW_SOCKET, LOW);
  delay(100);
  digitalWrite(PW_SW_SOCKET, HIGH);
  delay(200);
  digitalWrite(PW_SW_SOCKET, LOW);
  // Wait for ACPI shutdown
  unsigned long time = millis();
  while (read_pw1()) {
    if (millis() - time > ACPI_SHUTDOWN_TIMEOUT) {
      // force OFF
      time = millis();
      digitalWrite(PW_SW_SOCKET, HIGH);
      while (read_pw1()) {
        if (millis() - time > FORCE_SHUTDOWN_TIMEOUT) break;
        // else
        delay(100);
      }
      digitalWrite(PW_SW_SOCKET, LOW);
      break;
    }
    delay(100);
  }
}

void process_message(const char* message)
{
  Serial.print(F("Received: "));
  Serial.println(message);
  if (strlen(message) < 3 || strncmp_P(message, PSTR("OK\t"), 3) != 0) return;
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
      listen_rs485();
      put_register(0x9000/*Battery type*/, (uint16_t)battery_type);
      listen_wifi();
      Serial.print(F("Battery type saved: "));
      Serial.println(battery_type);
    } else if (strcmp(key, "bc") == 0) {
      int battery_capacity= atoi(value);
      listen_rs485();
      put_register(0x9001/*Battery capacity*/, (uint16_t)battery_capacity);
      listen_wifi();
      Serial.print(F("Battery capacity saved: "));
      Serial.print(battery_capacity);
      Serial.println(F("Ah"));
    } else if (strcmp(key, "pw") == 0 && isdigit(value[0])) { // main power
      int pw = atoi(value);
      if (pw == 0) {
        Serial.println(F("Power OFF"));
        if (read_pw1()) poweroff_pw1(); // atx power off
        poweroff_pw();
      } else if (pw == 1) {
        Serial.println(F("Power ON"));
        poweron_pw();
      }
    } else if (strcmp(key, "pw1") == 0 && (isdigit(value[0]) || value[0] == '-')) { // atx power
      int pw1 = atoi(value);
      bool pw1_on = read_pw1();
      if (pw1_on && pw1 == 0) { // power off
        Serial.println(F("Power1 OFF"));
        poweroff_pw1();
      } else if (!pw1_on && pw1 == 1) { // power on
        Serial.println(F("Power1 ON"));
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
      sprintf_P(buf, PSTR("Date saved: 20%02u-%02u-%02u %02u:%02u:%02u"), year, month, day, hour, minute, second);
      Serial.println(buf);
    }
  }

}

bool process_command_line(const char* line) // true = go to next line,  false = go to next loop
{
  LineParser lineparser(line);
  if (lineparser.get_count() == 0 || lineparser[0][0] =='#') return true;

  if (strcmp_P(lineparser[0], PSTR("exit")) == 0 || strcmp_P(lineparser[0], PSTR("quit")) == 0) {
    if (operation_mode == OPERATION_MODE_COMMAND_LINE_ONLY) {
      Serial.println(F("This is 'command line only' mode. make sure wifi config is right and restart system."));
      return true;
    }
    // else
    operation_mode = OPERATION_MODE_NORMAL;
    Serial.println(F("Exitting command line mode."));
    return false;
  } else if (strcmp_P(lineparser[0], PSTR("nodename")) == 0) {
    if (lineparser.get_count() < 2) {
      Serial.print(F("Current nodename is '"));
      Serial.print(config.nodename);
      Serial.println(F("'."));
      return true;
    }
    // else
    // TODO: validation
    strncpy(config.nodename, lineparser[1], sizeof(config.nodename));
    config.nodename[sizeof(config.nodename) - 1] = '\0'; // ensure null terminated as strncpy may not put it on tail
    Serial.print(F("Nodename set to '"));
    Serial.print(config.nodename);
    Serial.println(F("'. save and reboot the system to take effects."));
  } else if (strcmp_P(lineparser[0], PSTR("ssid")) == 0) {
    if (lineparser.get_count() < 2) {
      Serial.print(F("Current SSID is '"));
      Serial.print(config.ssid);
      Serial.println(F("'."));
      return true;
    }
    // else
    strncpy(config.ssid, lineparser[1], sizeof(config.ssid));
    config.ssid[sizeof(config.ssid) - 1] = '\0';
    Serial.print(F("SSID set to '"));
    Serial.print(config.ssid);
    Serial.println(F("'. save and reboot the system to take effects."));
  } else if (strcmp_P(lineparser[0], PSTR("key")) == 0) {
    if (lineparser.get_count() < 2) {
      Serial.print(F("Current WPA Key is '"));
      Serial.print(config.key);
      Serial.println(F("'."));
      return true;
    }
    // else
    strncpy(config.key, lineparser[1], sizeof(config.key));
    config.key[sizeof(config.key) - 1] = '\0';
    Serial.print(F("WPA Key set to '"));
    Serial.print(config.key);
    Serial.println(F("'. save and reboot the system to take effects."));
  } else if (strcmp_P(lineparser[0], PSTR("servername")) == 0) {
    if (lineparser.get_count() < 2) {
      Serial.print(F("Current Server name is '"));
      Serial.print(config.servername);
      Serial.println(F("'."));
      return true;
    }
    // else
    strncpy(config.servername, lineparser[1], sizeof(config.servername));
    config.servername[sizeof(config.servername) - 1] = '\0';
    Serial.print(F("Server name set to '"));
    Serial.print(config.servername);
    Serial.println(F("'. save and reboot the system to take effects."));
  } else if (strcmp_P(lineparser[0], PSTR("port")) == 0) {
    if (lineparser.get_count() < 2) {
      Serial.print(F("Current port is "));
      Serial.print(config.port);
      Serial.println('.');
      return true;
    }
    // else
    long port = atol(lineparser[1]);
    if (port < 1 || port > 65535) {
      Serial.println(F("Invalid port number."));
      return true;
    }
    // else
    config.port = (uint16_t)port;
    Serial.print(F("Server port set to "));
    Serial.print(port);
    Serial.println(F(". save and reboot the system to take effects."));
  } else if (strcmp_P(lineparser[0], PSTR("save")) == 0) {
    uint8_t* p = (uint8_t*)&config;
    config.crc = 0;
    for (size_t i = 0; i < sizeof(config) - sizeof(config.crc); i++) {
      config.crc = _crc16_update(config.crc, p[i]);
    }
    Serial.print(F("Writing config to EEPROM..."));
    EEPROM.put(0, config);

    Serial.println(F("Done."));
  } else if (strcmp_P(lineparser[0], PSTR("pw1")) == 0 && lineparser.get_count() > 1 && isdigit(lineparser[1][0])) {
    int pw = atoi(lineparser[1]);
    if (pw == 0) {
      Serial.println(F("Power1 OFF"));
      if (read_pw1()) poweroff_pw1(); // atx power off
    } else if (pw == 1) {
      Serial.println(F("Power1 ON"));
      poweron_pw1();
    }
  } else if (strcmp_P(lineparser[0], PSTR("wifidirect")) == 0) {
    if (operation_mode != OPERATION_MODE_COMMAND_LINE_ONLY) {
      Serial.println("Switching to WiFi Direct mode is available only in command line only mode.");
      return true;
    }
    uint32_t baudrate = 0;
    if (lineparser.get_count() > 1 && isdigit(lineparser[1][0])) {
      baudrate = atol(lineparser[1]);
    }
    Serial.println(F("Entering WiFi Direct mode. Ctrl-] to exit."));
    if (baudrate > 0 && baudrate >= 9600) {
      Serial.print(F("Setting baudrate to "));
      Serial.print(baudrate);
      Serial.println(F(" bps."));
      WiFi.begin(baudrate);
    }
    operation_mode = OPERATION_MODE_WIFI_DIRECT;
    return false;
  } else if (strcmp_P(lineparser[0], PSTR("?")) == 0 || strcmp_P(lineparser[0], PSTR("help")) == 0) {
    Serial.println(F("Available commands: nodename ssid key servername port exit"));
  } else if (strcmp_P(lineparser[0], PSTR("readcc")) == 0) {
    Serial.println("Reading values from charge controller...");

    EPSolarTracerInputRegister reg;
    char buf[128];
    if (get_register(0x3100, 6, reg)) {
      float piv = reg.getFloatValue(0);
      float pia = reg.getFloatValue(2);
      double piw = reg.getDoubleValue(4);
      float bv = reg.getFloatValue(8);
      float poa = reg.getFloatValue(10);
      strcpy_P(buf, PSTR("\tpiv:"));
      dtostrf(piv, 4, 2, buf + strlen(buf));
      strcat_P(buf, PSTR("\tpia:"));
      dtostrf(pia, 4, 2, buf + strlen(buf));
      strcat_P(buf, PSTR("\tpiw:"));
      dtostrf(piw, 4, 2, buf + strlen(buf));
      strcat_P(buf, PSTR("\tbv:"));
      dtostrf(bv, 4, 2, buf + strlen(buf));
      strcat_P(buf, PSTR("\tpoa:"));
      dtostrf(poa, 4, 2, buf + strlen(buf));
      Serial.println(buf);
    } else {
      Serial.println("Reading register failed.");
    }
  } else {
    Serial.println(F("Unrecognized command."));
  }
  return true;
}

void loop_command_line()
{
  int available = Serial.available();
  if (available > 0) {
    char buf[available];
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
        Serial.print(F("# "));
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
    Serial.println(F("Entering command line mode. '?' to help, 'exit' to exit."));
    Serial.print(F("# "));
    return;
  }

  unsigned long current_time = millis();
  if (current_time - last_report_time >= REPORT_INTERVAL) {
    listen_rs485();
    EPSolarTracerInputRegister reg;
    float piv,pia,bv,poa;
    double piw;
    double load;
    float temp, itemp;
    int cs;
    double lkwh;
    double kwh;
    int pw;

    bool success = false;

    if (get_register(0x3100, 6, reg)) {
      piv = reg.getFloatValue(0);
      pia = reg.getFloatValue(2);
      piw = reg.getDoubleValue(4);
      bv = reg.getFloatValue(8);
      poa = reg.getFloatValue(10);
      delay(20);
      if (get_register(0x310e, 4, reg)) {
        load = reg.getDoubleValue(0);
        temp = reg.getFloatValue(4);
        itemp = reg.getFloatValue(6);
        delay(20);
        if (get_register(0x3201, 1, reg)) { // Charging equipment status
          cs = (reg.getWordValue(0) >> 2) & 0x0003;
          delay(20);
          if (get_register(0x3304, 1, reg)) {
            lkwh = reg.getDoubleValue(0);
            delay(20);
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
    }

    listen_wifi();
    while (true) {
      char buf[64];
      if (success) {
        strcpy_P(buf, PSTR("DATA\tpiv:"));
        dtostrf(piv, 4, 2, buf + strlen(buf));
        WiFi.write(buf);

        strcpy_P(buf, PSTR("\tpia:"));
        dtostrf(pia, 4, 2, buf + strlen(buf));
        WiFi.write(buf);

        strcpy_P(buf, PSTR("\tpiw:"));
        dtostrf(piw, 4, 2, buf + strlen(buf));
        WiFi.write(buf);

        strcpy_P(buf, PSTR("\tbv:"));
        dtostrf(bv, 4, 2, buf + strlen(buf));
        WiFi.write(buf);

        strcpy_P(buf, PSTR("\tpoa:"));
        dtostrf(poa, 4, 2, buf + strlen(buf));
        WiFi.write(buf);

        strcpy_P(buf, PSTR("\tload:"));
        dtostrf(load, 4, 2, buf + strlen(buf));
        WiFi.write(buf);

        strcpy_P(buf, PSTR("\ttemp:"));
        dtostrf(temp, 4, 2, buf + strlen(buf));
        WiFi.write(buf);

        strcpy_P(buf, PSTR("\tlkwh:"));
        dtostrf(lkwh, 4, 2, buf + strlen(buf));
        WiFi.write(buf);

        strcpy_P(buf, PSTR("\tkwh:"));
        dtostrf(kwh, 4, 2, buf + strlen(buf));
        WiFi.write(buf);

        float btcv = 0.0f;
        if (battery_rated_voltage && temperature_compensation_coefficient && temp < 25.0f && piv >= bv && cs > 0) {
          btcv = 0.001 * temperature_compensation_coefficient * (25.0f - temp) * (battery_rated_voltage / 2);
        }
        strcpy_P(buf, PSTR("\tbtcv:"));
        dtostrf(btcv, 4, 2, buf + strlen(buf));
        WiFi.write(buf);

        sprintf_P(buf, PSTR("\tcs:%d"), cs);
        WiFi.write(buf);

        sprintf_P(buf, PSTR("\tpw:%d"), pw);
        sprintf_P(buf + strlen(buf), PSTR("\tpw1:%d"), read_pw1()? 1 : 0);
        WiFi.write(buf);
      } else {
        WiFi.write("NODATA");
      }

      if (session_id[0]) {
        strcpy_P(buf, PSTR("\tsession:"));
        strcat(buf, session_id);
        WiFi.write(buf);
      }

      strcpy_P(buf, PSTR("\tnodename:"));
      strcat(buf, config.nodename);

      if (send_message(buf)) break;
      //else
      // Perform autoreconnect when something fails
      Serial.println(F("Connection error. performing autoreconnect..."));
      delay(970);
      connect();
    }

    last_report_time = current_time;
  }
}

void loop_wifi_direct()
{
  while(WiFi.available()) Serial.write(WiFi.read());
  while(Serial.available()) {
    char c = Serial.read();
    if (c == 0x1d/*Ctrl-]*/) {
      Serial.print("Returning to command line.\r\n# ");
      operation_mode = OPERATION_MODE_COMMAND_LINE_ONLY;
      return;
    }
    // else
    WiFi.write(c);
    if (c == '\r') WiFi.write('\n');
  }
}

void loop()
{
  read_pw1();
  switch (operation_mode) {
    case OPERATION_MODE_NORMAL:
      loop_normal();
      break;
    case OPERATION_MODE_COMMAND_LINE:
    case OPERATION_MODE_COMMAND_LINE_ONLY:
      loop_command_line();
      break;
    case OPERATION_MODE_WIFI_DIRECT:
      loop_wifi_direct();
    default:
      break;
  }
}
