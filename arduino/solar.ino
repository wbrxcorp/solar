#include <SoftwareSerial.h>
#include <EEPROM.h>
#include <util/crc16.h>

//#define DEBUG_AT_COMMANDS

SoftwareSerial WiFi(10, 11); // RX, TX

static const int SW_SOCKET = 2;
static const int PW_SW_SOCKET = 3;
static const int PW_LED_SOCKET = 4;
static const int LED_SOCKET = 5;
static const int REPORT_INTERVAL = 5000;
static const size_t COMM_BUF_MAX = 64;   // set max line size to serial's internal buffer size

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
        sprintf(buf, "NODENAME\t%s\n", config.nodename);
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
  pinMode(SW_SOCKET, INPUT_PULLUP); // ON BOARD SWITCH
  pinMode(PW_SW_SOCKET, OUTPUT); // PC PWR BTN
  pinMode(PW_LED_SOCKET, INPUT_PULLUP); // PC PWR LED
  pinMode(LED_SOCKET, OUTPUT); // ON BOARD LED

  Serial.begin(9600);

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
  issue_at_command("AT+CWQAP");
  issue_at_command("AT+CWMODE_CUR=1");
  connect();
}

void process_message(const char* message)
{
  Serial.write("Received: ");
  Serial.println(message);
}

void loop()
{
  // ATX power control
  digitalWrite(PW_SW_SOCKET, digitalRead(SW_SOCKET)? LOW : HIGH); // digitalRead(*):  OFF=1 ON=0
  digitalWrite(LED_SOCKET, digitalRead(PW_LED_SOCKET)? LOW : HIGH);

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

    char buf[COMM_BUF_MAX];
    sprintf(buf, "DATA\ttime:%ld\n", current_time);
    send_textdata_with_autoreconnect(buf);
    last_report_time = current_time;
  }
}
