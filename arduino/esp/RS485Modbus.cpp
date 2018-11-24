#include "RS485Modbus.h"
#include "crc.h"

void RS485Modbus::begin(int _commPin, int _rtsPin, int _rtrPin, long speed, int _modbusTimeout)
{
  commPin = _commPin;
  m_bitTime = F_CPU / speed;
  modbusTimeout = F_CPU / 1000 * _modbusTimeout;
  rtsPin = _rtsPin;
  rtrPin = _rtrPin;
  digitalWrite(rtsPin, LOW);  // disable RS485 driver
  pinMode(rtsPin, OUTPUT);
  if (rtrPin >= 0) {
    digitalWrite(rtrPin, HIGH); // disable RS485 receiver
    pinMode(rtrPin, OUTPUT);
  }
}

#define WAIT { while (ESP.getCycleCount()-start < wait) { ; }; wait += m_bitTime; }

void RS485Modbus::send_modbus_message(const uint8_t* message, size_t size)
{
  unsigned long startTime = ESP.getCycleCount();
  while (ESP.getCycleCount() - startTime < m_bitTime * 10 * 7 / 2/*3.5 chars silent interval*/) { ; }

  // set commPin to output mode
  digitalWrite(commPin, HIGH);
  pinMode(commPin, OUTPUT);

  if (rtrPin >= 0) digitalWrite(rtrPin, HIGH); // disable RS485 receiver
  digitalWrite(rtsPin, HIGH); // enable RS485 driver

  for (int i = 0; i < size; i++) {
    cli(); // Disable interrupts in order to get a clean transmit
    uint8_t b = message[i];
    unsigned long wait = m_bitTime;
    digitalWrite(commPin, HIGH);
    unsigned long start = ESP.getCycleCount();
    // Start bit;
    digitalWrite(commPin, LOW);
    WAIT;
    for (int j = 0; j < 8; j++) {
      digitalWrite(commPin, (b & 1) ? HIGH : LOW);
      WAIT;
      b >>= 1;
    }
    // Stop bit
    digitalWrite(commPin, HIGH);
    WAIT;
    sei();
  }

  digitalWrite(rtsPin, LOW); // disable RS485 driver

  // turn commPin back to input
  pinMode(commPin, INPUT);

  startTime = ESP.getCycleCount();
  while (ESP.getCycleCount() - startTime < m_bitTime * 10 * 7 / 2/*3.5 chars silent interval*/) { ; }
}

int ICACHE_RAM_ATTR RS485Modbus::receive_modbus_message(uint8_t* modbus_message)
{
  int message_size;
  bool waitingForFirstBit = true;
  cli();

  if (rtrPin >= 0) digitalWrite(rtrPin, LOW); // enable RS485 receiver

  unsigned long startTime = ESP.getCycleCount();
  while (ESP.getCycleCount() - startTime < m_bitTime * 10 * 7 / 2 /*ignore input for some chars as turning RS485 receiver on produces some garbage when bias resistor is not connected*/) { ; }

  for (message_size = 0; message_size < MAX_MODBUS_MESSAGE_LENGTH; message_size++) {
    startTime = ESP.getCycleCount();
#ifdef ARDUINO_ARCH_ESP8266
    while (GPIP(commPin)) { // wait for a start bit
#else
    while(digitalRead(commPin) == HIGH) { // wait for a start bit
#endif
      if ((ESP.getCycleCount() - startTime) > (waitingForFirstBit? modbusTimeout : (m_bitTime * 10 * 7 / 2)/*3.5 chars silent interval*/)) goto out;
#ifdef ARDUINO_ARCH_ESP8266
      ESP.wdtFeed();
#endif
    }
    waitingForFirstBit = false;
    unsigned long wait = m_bitTime + m_bitTime / 2;
    unsigned long start = ESP.getCycleCount();
    uint8_t rec = 0;
    for (int j = 0; j < 8; j++) {
      WAIT;
      rec >>= 1;
#ifdef ARDUINO_ARCH_ESP8266
      if (GPIP(commPin)) rec |= 0x80;
#else
      if (digitalRead(commPin)) rec |= 0x80;
#endif
    }
    // Stop bit
    WAIT;
    modbus_message[message_size] = rec;
  }
out:;
  if (rtrPin >= 0) digitalWrite(rtrPin, HIGH); // disable RS485 receiver

  sei();

  if (message_size == 0) {
    Serial.println("modbus: Response timeout(No device connected?)");
    return 0;
  }

  if (message_size < 4) {
    Serial.println("modbus: Response message too short(expected at least 4bytes)");
    return -1;
  }

  uint16_t crc = 0xffff;
  for (int i = 0; i < message_size - 2; i++) crc = update_crc(crc, modbus_message[i]);
  if (modbus_message[message_size - 2] != LOBYTE(crc) || modbus_message[message_size - 1] != HIBYTE(crc)) {
    Serial.println("modbus: CRC mismatch");
    return -1;
  }

  return message_size;
}
