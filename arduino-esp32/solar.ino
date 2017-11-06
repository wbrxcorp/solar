// arduino --upload --board espressif:esp32:esp32doit-devkit-v1:FlashFreq=80,UploadSpeed=115200 --port /dev/ttyUSB0 solar.ino
#include <EEPROM.h>
#include <WiFi.h>
#include <ESPmDNS.h>

#define UART0_RX_SOCKET
#define UART0_TX_SOCKET
#define SOCKET_26
#define SOCKET_25
#define UART2_TX_SOCKET 17
#define UART2_RX_SOCKET 16
#define RS485_RTS_SOCKET 27
#define SOCKET_14

#define PW1_SW_SOCKET 12
#define PW1_LED_SOCKET 13
#define PW2_SW_SOCKET 5
#define PW2_LED_SOCKET 23
#define PW_IND_LED_SOCKET 19
#define COMMAND_LINE_ONLY_MODE_SOCKET 18

#define OPERATION_MODE_NORMAL 0
#define OPERATION_MODE_COMMAND_LINE 1
#define OPERATION_MODE_COMMAND_LINE_ONLY 2

#define COMM_BUF_MAX 128

const uint16_t DEFAULT_PORT = 29574; // default server port number

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
  bool push(const uint8_t* bytes, size_t len) {
    for (int i = 0; i < len; i++) {
      if (size < max_size - 1) buf[size++] = (char)bytes[i];
      if (bytes[i] == this->terminator && size == max_size) buf[max_size - 1] = this->terminator;
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

HardwareSerial RS485(2);  // RX=16,TX=17
WiFiClient tcp_client;

uint8_t operation_mode = OPERATION_MODE_NORMAL;

struct {
  char nodename[32];
  char ssid[34];
  char key[64];
  char servername[48];
  uint16_t port;
  uint16_t crc;
} config;

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

void connect()
{
  unsigned long retry_delay = 1;/*sec*/
  while (true) {
    Serial.print("Connecting to ");
    Serial.print(config.servername);
    Serial.print(':');
    Serial.print(config.port);
    Serial.println("...");
    int connected = 0;
    int servername_len = strlen(config.servername);
    if (servername_len > 6 && strcmp(config.servername + servername_len - 6, ".local") == 0) {
      // query mDNS
      Serial.print("Querying mDNS...");
      MDNSResponder mdns;
      if (mdns.begin(config.nodename, TCPIP_ADAPTER_IF_STA, 300)) {
        char servername_without_suffix[servername_len - 5];
        strncpy(servername_without_suffix, config.servername, servername_len - 6);
        servername_without_suffix[servername_len - 6] = '\0';
        IPAddress addr = mdns.queryHost(servername_without_suffix);
        mdns.end();
        if (addr != INADDR_NONE)  {
          Serial.println(addr.toString());
          connected = tcp_client.connect(addr, config.port);
        } else {
          Serial.println("Failed to obtain server IP address via mDNS.");
        }
      } else {
        Serial.println("Failed to init mDNS.");
      }
    } else {
      connected = tcp_client.connect(config.servername, config.port);
    }
    if (connected) {
      Serial.println("Connected.");
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

void setup() {
  Serial.begin(115200);
  pinMode(RS485_RTS_SOCKET, OUTPUT);
  pinMode(PW1_SW_SOCKET, OUTPUT);
  pinMode(PW1_LED_SOCKET, INPUT_PULLUP);
  pinMode(PW2_SW_SOCKET, OUTPUT);
  pinMode(PW2_LED_SOCKET, INPUT_PULLUP);
  pinMode(PW_IND_LED_SOCKET, OUTPUT);
  pinMode(COMMAND_LINE_ONLY_MODE_SOCKET, INPUT_PULLUP); // Short to enter command line only mode

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

  Serial.print("Connecting to WiFi AP");
  WiFi.setAutoReconnect(true);
  WiFi.begin(config.ssid, config.key);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" Connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  connect();
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
    config.crc = 0xffff;
    for (size_t i = 0; i < sizeof(config) - sizeof(config.crc); i++) {
      config.crc = update_crc(config.crc, p[i]);
    }
    Serial.print(F("Writing config to EEPROM..."));
    EEPROM.begin(sizeof(config));
    EEPROM.put(0, config);
    EEPROM.commit();
    EEPROM.end();

    Serial.println(F("Done."));
  } else if (strcmp_P(lineparser[0], PSTR("?")) == 0 || strcmp_P(lineparser[0], PSTR("help")) == 0) {
    Serial.println(F("Available commands: nodename ssid key servername port exit"));
  } else {
    Serial.println(F("Unrecognized command."));
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
  Serial.print("TCP client connected status:");
  Serial.println((int)tcp_client.connected());
  if (!tcp_client.connected()) {
    Serial.println("Recovering connection.");
    connect();
  }
  digitalWrite(PW_IND_LED_SOCKET, HIGH);
  delay(500);
  digitalWrite(PW_IND_LED_SOCKET, LOW);
  delay(500);
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
