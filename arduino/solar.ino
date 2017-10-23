#include <SoftwareSerial.h>
#include <EEPROM.h>
#include <util/crc16.h>

#define HIBYTE(word) ((uint8_t)((word & long(255*256L)) >> 8))
#define LOBYTE(word) ((uint8_t)(word & 255))

//#define DEBUG_AT_COMMANDS

static const int WIFI_RX_SOCKET = 2;
static const int WIFI_TX_SOCKET = 3;

static const int PW_SW_SOCKET = 4;
static const int PW_LED_SOCKET = 5;

static const int RS485_RX_SOCKET = 6;
static const int RS485_TX_SOCKET = 7;
static const int RS485_RTS_SOCKET = 8;

static const int REPORT_INTERVAL = 5000;
static const size_t COMM_BUF_MAX = 128;   // set max line size to serial's internal buffer size

SoftwareSerial WiFi(WIFI_RX_SOCKET, WIFI_TX_SOCKET); // RX, TX
SoftwareSerial RS485(RS485_RX_SOCKET, RS485_TX_SOCKET, 0); // RX, TX

char line_buffer[COMM_BUF_MAX];
char receive_buffer[COMM_BUF_MAX];
size_t receive_buffer_len = 0;

unsigned long last_report_time = 0;

struct {
  char nodename[32];
  char ssid[34];
  char key[64];
  char servername[48];
  uint16_t port;
  uint16_t crc;
} config;

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
    if (_data) delete []_data;
    _data = new uint8_t[size];
    memcpy(_data, data, size);
    _size = size;
  }
  size_t size() const { return _size; }
  const uint8_t* data() const { return _data; }

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
    return year * 10000000000 + month * 100000000 + day * 1000000L
      + hour * 10000L + minute * 100 + second;
  }
};

char* readline()
{
  int len = 0;
  ExpectedCharSeq term = ExpectedCharSeq("\r\n");

  while (true) {
    while (WiFi.available()) {
      char c = WiFi.read();
#ifdef DEBUG_AT_COMMANDS
        Serial.write(c);
#endif
      if (len < sizeof(line_buffer) - 1) line_buffer[len++] = c;
      if (term.push(c)) goto out;
    }
    delay(100); // wait for next batch of chars
  }
out:;
  line_buffer[len - 2] = '\0'; // overwrite '\r' with terminater
  return line_buffer;
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
  while (true) {
    int available = WiFi.available();
    if (available > 0) {
      for (int i = 0; i < available; i++) {
        char c = WiFi.read();
#ifdef DEBUG_AT_COMMANDS
        Serial.write(c);
#endif
        if (success.push(c)) return true;
        if (fail.push(c)) return false;
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

bool send_textdata(const char* data)
{
  char buf[19];
  sprintf(buf, "AT+CIPSEND=%d\r\n", strlen(data));
  WiFi.write(buf);
  if (wait_for_result("\r\nOK\r\n> ", "\r\nERROR\r\n")) {
    WiFi.write(data, strlen(data));
    return wait_for_result("\r\nSEND OK\r\n", "\r\nERROR\r\n");
  } else return false;
}

void connect()
{
  int retry_delay = 1;/*sec*/
  char buf[128];
  while (true) {
    Serial.println("Connecting to WiFi AP...");
    strcpy(buf, "AT+CWJAP_CUR=\"");
    strcat(buf, config.ssid);
    strcat(buf, "\",\"");
    strcat(buf, config.key);
    strcat(buf, "\"");
    if (issue_at_command(buf, "\r\nFAIL\r\n")) {
      Serial.println("Connecting to the server...");
      sprintf(buf, "AT+CIPSTART=\"TCP\",\"%s\",%d,10", config.servername, config.port);
      if (issue_at_command(buf)) {
        sprintf(buf, "INIT\tnodename:%s\n", config.nodename);
        if (send_textdata(buf)) {
          Serial.println("Connection established.");
          return;
        }
      } else {
        // "ERROR"
        wait_for("CLOSED\r\n");
        Serial.println("Connecting to server failed.");
      }
    } else {
      Serial.println("Connecting to WiFi AP failed.");
    }
    // retry
    Serial.println("Performing retry...");
    delay(retry_delay * 1000);
    retry_delay *= 2;
    if (retry_delay > 60) retry_delay = 60;
  }
}

void send_textdata_with_autoreconnect(const char* data)
{
  int reconnect_delay = 1; /*sec*/
  while (!send_textdata(data)) {
    Serial.println("Connection error. performing autoreconnect...");
    // retry
    delay(reconnect_delay * 1000);
    connect();
    reconnect_delay *= 2;
    if (reconnect_delay > 60) reconnect_delay = 60;
  }
}

size_t get_stream_length()
{
  if (!wait_for_result("\r\n+IPD,", "CLOSED\r\n")) return 0; // can be "CLOSED" when TCP session is lost
  int bytes = 0;
  while(true) {
    int available = WiFi.available();
    if (available > 0) {
      for (int i = 0; i < available; i++) {
        char c = WiFi.read();
#ifdef DEBUG_AT_COMMANDS
        Serial.write(c);
#endif
        if (c == ':') return (size_t)bytes;
        if (c >= '0' && c <= '9') {
          bytes *= 10;
          bytes += (c - '0');
        }
      }
    } else delay(100);
  }
}

void input_config()
{
  strcpy(config.nodename, "kennel01");
  strcpy(config.ssid, "YOUR_ESSID");
  strcpy(config.key, "YOUR_WPA_KEY");
  strcpy(config.servername, "your.server");
  config.port = 29574;
}

void setup() {
  pinMode(PW_SW_SOCKET, OUTPUT); // PC PWR BTN
  pinMode(PW_LED_SOCKET, INPUT_PULLUP); // PC PWR LED

  Serial.begin(9600);
  RS485.begin(115200);
  pinMode(RS485_RTS_SOCKET, OUTPUT);

  if (false/*TODO: check if reset jumper is close*/) {
    // break crc on purpose
    EEPROM.put(sizeof(config) - sizeof(config.crc), (uint16_t)0);
  }

  // read config from EEPROM
  Serial.write("Loading config from EEPROM...");
  EEPROM.get(0, config);

  uint8_t* p = (uint8_t*)&config;
  uint16_t crc = 0;
  for (size_t i = 0; i < sizeof(config) - sizeof(config.crc); i++) {
    crc = _crc16_update(crc, p[i]);
  }
  if (crc != config.crc) {
    Serial.write("Checksum mismatch(expected=");
    Serial.print(crc);
    Serial.write(", actual=");
    Serial.print(config.crc);
    Serial.println(")");

    memset(&config, 0, sizeof(config));
    input_config();
    config.crc = 0;
    for (size_t i = 0; i < sizeof(config) - sizeof(config.crc); i++) {
      config.crc = _crc16_update(config.crc, p[i]);
    }
    Serial.println("Writing config...");
    EEPROM.put(0, config);
  }
  Serial.println("Done.");
  Serial.write("Nodename: ");
  Serial.println(config.nodename);
  Serial.write("WiFi SSID: ");
  Serial.println(config.ssid);
  Serial.println("WiFi Password: *");
  Serial.write("Server name: ");
  Serial.println(config.servername);
  Serial.write("Server port: ");
  Serial.println(config.port);

  //WiFi.begin(115200);
  //WiFi.write("AT+UART_DEF=9600,8,1,0,0\r\n");
  WiFi.begin(9600);

  issue_at_command("AT");
  issue_at_command("AT+CWQAP");
  issue_at_command("AT+CWMODE_CUR=1");
  connect();
}

void process_message(const char* message)
{
  Serial.write("Received: ");
  Serial.println(message);
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

bool get_register(uint16_t addr, uint8_t num, EPSolarTracerInputRegister& reg, int max_retry = 10)
{
  uint8_t function_code = 0x04; // Read Input Register
  if (addr >= 0x9000 && addr < 0x9100) function_code = 0x03; // Read Holding Register

  byte message[] = {0x01, function_code, HIBYTE(addr), LOBYTE(addr), 0x00, num, 0x00, 0x00 };
  // calc crc
  uint16_t crc = 0xFFFF;
  for (int pos = 0; pos < sizeof(message) - 2; pos++) {
    crc = update_crc(crc, message[pos]);
  }
  message[sizeof(message) - 2] = LOBYTE(crc);
  message[sizeof(message) - 1] = HIBYTE(crc);

  for (int i = 0; i < max_retry; i++) {
    digitalWrite(RS485_RTS_SOCKET,HIGH);
    delay(1);
    RS485.write(message, sizeof(message));
    delay(1);
    digitalWrite(RS485_RTS_SOCKET,LOW);
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
              crc = update_crc(update_crc(update_crc(0xFFFF,hdr[0]), hdr[1]), hdr[2]);
              for (int i = 0; i < sizeof(buf); i++) crc = update_crc(crc, buf[i]);
              if (rx_crc[0] == LOBYTE(crc) && rx_crc[1] == HIBYTE(crc)) {
                reg.setData(buf, data_size);
                return true;
              }
            }
          }
        }
      }
    }
    // else
    while (RS485.available()) RS485.read(); // discard remaining bytes
    delay(50);
  }
  return false;
}

void loop()
{
  // ATX power control
  //digitalWrite(PW_SW_SOCKET, digitalRead(SW_SOCKET)? LOW : HIGH); // digitalRead(*):  OFF=1 ON=0
  //digitalWrite(LED_SOCKET, digitalRead(PW_LED_SOCKET)? LOW : HIGH);

  // process messages if any data is in serial input buffer
  while (WiFi.available()) {
    size_t stream_length = get_stream_length();
    if (stream_length == 0) break; // something other than stream from server detected
    char buf[stream_length]; // Oh No.
    int r = WiFi.readBytes(buf, sizeof(buf));
    for (int i = 0; i < r; i++) {
      char c = buf[i];
      receive_buffer[receive_buffer_len] = c;
      if (receive_buffer_len < sizeof(receive_buffer) - 2 && c != '\n') {
        receive_buffer_len++;
      } else if (receive_buffer_len >= sizeof(receive_buffer) - 2) {
        Serial.write("Receive buffer exceeded!");
      }
      receive_buffer[receive_buffer_len] = '\0';

      if (c == '\n') {
        receive_buffer_len = 0;
        process_message(receive_buffer);
      }
    }
  }

  unsigned long current_time = millis();
  if (current_time - last_report_time >= REPORT_INTERVAL) {
    /*
    WiFi.write("AT+CWJAP_CUR?\r\n");
    wait_for("+CWJAP_CUR:");
    const char* line = readline();
    const char* pt = line;
    int comma_cnt = 0;
    while (comma_cnt < 3) {
      if (*pt == ',') comma_cnt++;
      pt++;
    }
    char rssi[strlen(pt)];
    strcpy(pt, rssi);
    wait_for("OK\r\n");
    */

    /*"Charging equipment input voltage": 0x3100
      "Charging equipment input current": 0x3101
      "Charging equipment input power": 0x3102, 0x3103
      "Charging equipment output voltage": 0x3104,
      "Charging equipment output current": 0x3105,
      "Discharging equipment output power": 0x310E, 0x310F
      "Battery Temperature": 0x3110
      "Consumed energy today": 0x3304,0x3305
      "Generated energy today": 0x330C, 0x330D
    */
    RS485.listen();
    EPSolarTracerInputRegister reg;
    float piv,pia,bv,poa;
    double piw;
    double load;
    float temp;
    double lkwh;
    double kwh;
    char buf[COMM_BUF_MAX] = "NODATA\n";

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

            strcpy(buf, "DATA\tpiv:");
            dtostrf(piv, 4, 2, buf + strlen(buf));
            strcat(buf, "\tpia:");
            dtostrf(pia, 4, 2, buf + strlen(buf));
            strcat(buf, "\tpiw:");
            dtostrf(piw, 4, 2, buf + strlen(buf));
            strcat(buf, "\tbv:");
            dtostrf(bv, 4, 2, buf + strlen(buf));
            strcat(buf, "\tpoa:");
            dtostrf(poa, 4, 2, buf + strlen(buf));
            strcat(buf, "\tload:");
            dtostrf(load, 4, 2, buf + strlen(buf));
            strcat(buf, "\ttemp:");
            dtostrf(temp, 4, 2, buf + strlen(buf));
            strcat(buf, "\tlkwh:");
            dtostrf(lkwh, 4, 2, buf + strlen(buf));
            strcat(buf, "\tkwh:");
            dtostrf(kwh, 4, 2, buf + strlen(buf));
            strcat(buf, "\n");
          }
        }
      }
    }

#if 0
    uint64_t rtc;
    if (get_register(0x9013/*Real Time Clock*/, 3, reg)) {
      rtc = reg.getRTCValue(0);
    }
#endif

    WiFi.listen();
    //sprintf(buf, "DATA\ttime:%ld\n", current_time);
    send_textdata_with_autoreconnect(buf);

    last_report_time = current_time;
  }
}
