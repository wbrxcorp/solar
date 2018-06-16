// arduino --upload --board arduino:avr:mega:cpu=atmega2560 --port /dev/ttyACM0 .

#define HIBYTE(word) ((uint8_t)((word & 0xff00) >> 8))
#define LOBYTE(word) ((uint8_t)(word & 0xff))

#define RS485_RTS_SOCKET 2 // .. DE-RE
#define RS485 Serial1 // RX=19,TX=18

uint8_t buf[1024];
uint8_t* p = buf;
unsigned long last_byte_time;

#define STATE_IDLE 0
#define STATE_REQUEST 1
#define STATE_RESPONSE 2

int status = STATE_IDLE;

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

void print_bytes(const uint8_t* bytes, size_t size)
{
  for (int i = 0; i < size; i++) {
    char buf[8];
    sprintf(buf, "%02x ", bytes[i]);
    Serial.print(buf);
  }
  Serial.println();
}

void setup()
{
  Serial.begin(115200);

  Serial.println("Starting to snoop");

  last_byte_time = micros();
  uint8_t* p = buf;
  int status = STATE_IDLE;
  pinMode(RS485_RTS_SOCKET, OUTPUT);
  digitalWrite(RS485_RTS_SOCKET,LOW);
  RS485.begin(115200);
}

void serialEvent1()
{
  if (status == STATE_IDLE) {
    status = STATE_REQUEST;
  }
  int available;
  while ((available = RS485.available()) > 0) {
    int size = RS485.readBytes(p, available);// TODO: consider boundary
    p += size;
  }
  last_byte_time = micros();
}

void loop()
{
  unsigned long time_past = micros() - last_byte_time;
  if (time_past  >= 608 && p > buf) {
    if (status == STATE_REQUEST) {
      Serial.print(">>> ");
      status = STATE_RESPONSE;
    } else if (status == STATE_RESPONSE) {
      Serial.print("<<< ");
      status = STATE_IDLE;
    }
    print_bytes(buf, p - buf);
    p = buf;
  } else if (time_past >= 100000) {
    status = STATE_IDLE;
  }
}
