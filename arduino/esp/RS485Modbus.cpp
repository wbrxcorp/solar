#include "RS485Modbus.h"
#include "crc.h"

#if defined(ARDUINO_ARCH_ESP8266)
void RS485Modbus::begin(int _commPin, int _rtsPin, int _rtrPin, uint32_t _speed, int _modbusTimeout)
{
  commPin = _commPin;

  speed = _speed;
  //m_bitTime = F_CPU / speed;
  modbusTimeoutInMs = _modbusTimeout;//  = F_CPU / 1000 * _modbusTimeout;
  rtsPin = _rtsPin;
  rtrPin = _rtrPin;
  digitalWrite(rtsPin, LOW);  // disable RS485 driver
  pinMode(rtsPin, OUTPUT);
  if (rtrPin >= 0) {
    digitalWrite(rtrPin, HIGH); // disable RS485 receiver
    pinMode(rtrPin, OUTPUT);
  }
}
#elif defined(ARDUINO_ARCH_ESP32)
void RS485Modbus::begin(uart_port_t _uartNum, int txPin, int rxPin, int rtsPin, int _rtrPin/*-1 not to use*/, uint32_t _speed, int _modbusTimeout)
{
  uartNum = _uartNum;
  //m_bitTime = F_CPU / speed;
  speed = _speed;
  modbusTimeoutInMs = _modbusTimeout;//  = F_CPU / 1000 * _modbusTimeout;
  //modbusTimeout = F_CPU / 1000 * _modbusTimeout;
  rtrPin = _rtrPin;

  if (rtrPin >= 0) {
    digitalWrite(rtrPin, HIGH); // disable RS485 receiver
    pinMode(rtrPin, OUTPUT);
  }

  uart_config_t uart_config = {
    .baud_rate = (int)speed,
    .data_bits = UART_DATA_8_BITS,
    .parity = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
  };
  uart_param_config(uartNum, &uart_config);
  uart_set_pin(uartNum, txPin, rxPin, rtsPin, UART_PIN_NO_CHANGE);
  uart_driver_install(uartNum, MAX_MODBUS_MESSAGE_LENGTH + 1, 0, 0, NULL, 0);
  uart_set_mode(uartNum, UART_MODE_RS485_HALF_DUPLEX);
}
#endif

void RS485Modbus::enter_slave()
{
  slave = true;
#ifdef ARDUINO_ARCH_ESP8266
  digitalWrite(rtsPin, LOW); // disable RS485 driver
#endif
  if (rtrPin >= 0) digitalWrite(rtrPin, LOW); // enable RS485 driver
}

#define WAIT { while (ESP.getCycleCount()-start < wait) { ; }; wait += m_bitTime; }

void RS485Modbus::send_modbus_message(const uint8_t* message, size_t size)
{
  uint32_t m_bitTime = F_CPU / speed;
  uint32_t startTime = ESP.getCycleCount();
  while (ESP.getCycleCount() - startTime < m_bitTime * 10 * 7 / 2/*3.5 chars silent interval*/) { ; }

#if defined(ARDUINO_ARCH_ESP8266)
  // set commPin to output mode
  digitalWrite(commPin, HIGH);
  pinMode(commPin, OUTPUT);

  if (rtrPin >= 0) digitalWrite(rtrPin, HIGH); // disable RS485 receiver
  digitalWrite(rtsPin, HIGH); // enable RS485 driver

  for (int i = 0; i < size; i++) {
    cli(); // Disable interrupts in order to get a clean transmit
    uint8_t b = message[i];
    uint32_t wait = m_bitTime;
    uint32_t start = ESP.getCycleCount();
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
  if (rtrPin >= 0 && slave) digitalWrite(rtrPin, LOW); // enable RS485 receiver

  // turn commPin back to input
  pinMode(commPin, INPUT);

#elif defined(ARDUINO_ARCH_ESP32)
  if (rtrPin >= 0) digitalWrite(rtrPin, HIGH); // disable RS485 receiver
  uart_write_bytes(uartNum, (const char*)message, size);
  if (rtrPin >= 0 && slave) digitalWrite(rtrPin, LOW); // enable RS485 receiver
#endif

  if (!slave) {
    startTime = ESP.getCycleCount();
    while (ESP.getCycleCount() - startTime < m_bitTime * 10 * 7 / 2/*3.5 chars silent interval*/) { ; }
  }
}

int ICACHE_RAM_ATTR RS485Modbus::receive_modbus_message(uint8_t* modbus_message)
{
  int message_size;
  bool waitingForFirstBit = true;
  uint32_t m_bitTime = F_CPU / speed;
  uint32_t modbusTimeout = F_CPU / 1000 * modbusTimeoutInMs;
  if (rtrPin >= 0 && !slave) digitalWrite(rtrPin, LOW); // enable RS485 receiver

  uint32_t startTime = ESP.getCycleCount();
  //ignore input for some chars as turning RS485 receiver on produces some garbage when bias resistor is not connected
  if (!slave) {
    while (ESP.getCycleCount() - startTime < m_bitTime * 10 * 7 / 2/* 3.5 chars as modbus protocol defines*/) { ; }
  }

#if defined(ARDUINO_ARCH_ESP8266)
  cli();

  for (message_size = 0; message_size < MAX_MODBUS_MESSAGE_LENGTH; message_size++) {
    startTime = ESP.getCycleCount();
    while (GPIP(commPin)) { // wait for a start bit
      if ((ESP.getCycleCount() - startTime) > (waitingForFirstBit? modbusTimeout : (m_bitTime * 10 * 7 / 2)/*3.5 chars silent interval*/)) goto out;
      ESP.wdtFeed();
    }
    uint32_t wait = m_bitTime + m_bitTime / 2;
    uint32_t start = ESP.getCycleCount();
    uint8_t rec = 0;
    for (int j = 0; j < 8; j++) {
      WAIT;
      rec >>= 1;
      if (GPIP(commPin)) rec |= 0x80;
    }
    // Stop bit
    WAIT;
    waitingForFirstBit = false;
    modbus_message[message_size] = rec;
  }
out:;
  sei();

#elif defined(ARDUINO_ARCH_ESP32)
  if (!slave) uart_flush(uartNum);
  message_size = 0;
  startTime = ESP.getCycleCount();

  while (message_size < MAX_MODBUS_MESSAGE_LENGTH) {
    int r;
    if (waitingForFirstBit) {
      r = uart_read_bytes(uartNum, modbus_message + message_size, 1, modbusTimeoutInMs / portTICK_RATE_MS);
      waitingForFirstBit = false;
      startTime = ESP.getCycleCount();
    } else {
      uint32_t currentTime = ESP.getCycleCount();
      if (currentTime - startTime > m_bitTime * 10 * 7 / 2/*3.5 chars silent interval*/) break; // timeout
      size_t rxbufdatasz;
      if (uart_get_buffered_data_len(uartNum, &rxbufdatasz) != ESP_OK) {
        Serial.println("Serial port error(uart_get_buffered_data_len)");
        return -1;
      }
      // else
      if (rxbufdatasz < 1) continue;
      // else
      r = uart_read_bytes(uartNum, modbus_message + message_size, min(rxbufdatasz, (size_t)(MAX_MODBUS_MESSAGE_LENGTH - message_size)), 0);
      startTime = currentTime;
    }
    if (r < 0) {
      Serial.println("RS485 failed to receive");
      return -1;
    }
    // else
    if (r == 0) break;
    // else
    message_size += r;
  }
#endif

  if (rtrPin >= 0 && !slave) digitalWrite(rtrPin, HIGH); // disable RS485 receiver

  if (message_size == 0) {
    if (!slave) Serial.println("modbus: Response timeout(No device connected?)");
    return 0;
  }

  if (message_size < 4) {
    Serial.println("modbus: Response message too short(expected at least 4bytes)");
    return -1;
  }

  uint16_t crc = 0xffff;
  for (int i = 0; i < message_size - 2; i++) crc = update_crc(crc, modbus_message[i]);
  if (modbus_message[message_size - 2] != LOBYTE(crc) || modbus_message[message_size - 1] != HIBYTE(crc)) {
    Serial.print("modbus: CRC mismatch. Message=[ ");
    char buf[4];
    for (int i = 0; i < message_size; i++) {
      sprintf(buf, "%02x ", modbus_message[i]);
      Serial.print(buf);
    }
    Serial.print("](");
    Serial.print(message_size);
    Serial.println(" bytes)");

    return -1;
  }
  return message_size;
}
