/*
OneWireHalfDuplexSoftwareSerial.cpp - espsoftwareserial modified for limited purpose

-- The original license terms below

SoftwareSerial.h

SoftwareSerial.cpp - Implementation of the Arduino software serial for ESP8266.
Copyright (c) 2015-2016 Peter Lerup. All rights reserved.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifdef ARDUINO_ARCH_ESP8266

#include <Arduino.h>

// The Arduino standard GPIO routines are not enough,
// must use some from the Espressif SDK as well
extern "C" {
#include "gpio.h"
}

#include "OneWireHalfDuplexSoftwareSerial.h"

OneWireHalfDuplexSoftwareSerial* OneWireHalfDuplexSoftwareSerial::theInstance = NULL;

OneWireHalfDuplexSoftwareSerial* OneWireHalfDuplexSoftwareSerial::getInstance(unsigned int buffSize) {
  if (!theInstance) theInstance = new OneWireHalfDuplexSoftwareSerial(buffSize);
  return theInstance;
}

OneWireHalfDuplexSoftwareSerial::OneWireHalfDuplexSoftwareSerial(unsigned int buffSize) : m_overflow(false), m_rxEnabled(false), m_buffSize(buffSize) {
  // setup rx
  m_buffer = (uint8_t*)malloc(buffSize);
  if (m_buffer != NULL) {
    m_inPos = m_outPos = 0;
    pinMode(communicationPin, INPUT);
  }

  // Default speed
  begin(9600);
}

OneWireHalfDuplexSoftwareSerial::~OneWireHalfDuplexSoftwareSerial() {
  enableRx(false);
  theInstance = NULL;
  if (m_buffer) free(m_buffer);
}

void OneWireHalfDuplexSoftwareSerial::begin(long speed) {
  // Use getCycleCount() loop to get as exact timing as possible
  m_bitTime = F_CPU/speed;
  // By default enable interrupt during tx only for low speed
  m_intTxEnabled = speed < 9600;

  if (!m_rxEnabled) enableRx(true);
}

long OneWireHalfDuplexSoftwareSerial::baudRate() {
  return F_CPU / m_bitTime;
}

void OneWireHalfDuplexSoftwareSerial::enableIntTx(bool on) {
  m_intTxEnabled = on;
}

void OneWireHalfDuplexSoftwareSerial::enableTx(bool on) {
  if (on) {
    enableRx(false);
    digitalWrite(communicationPin, HIGH);
    pinMode(communicationPin, OUTPUT);
  } else {
    digitalWrite(communicationPin, HIGH);
    pinMode(communicationPin, INPUT);
    enableRx(true);
  }
}

void OneWireHalfDuplexSoftwareSerial::enableRx(bool on) {
  if (!isRxValid()) return;
  // else
  if (on)
    attachInterrupt(communicationPin, OneWireHalfDuplexSoftwareSerial::rxRead, FALLING);
  else
    detachInterrupt(communicationPin);
  m_rxEnabled = on;
}

int OneWireHalfDuplexSoftwareSerial::read() {
  if (!isRxValid() || (m_inPos == m_outPos)) return -1;
  // else
  uint8_t ch = m_buffer[m_outPos];
  m_outPos = (m_outPos+1) % m_buffSize;
  return ch;
}

int OneWireHalfDuplexSoftwareSerial::available() {
  if (!isRxValid()) return 0;
  // else
  int avail = m_inPos - m_outPos;
  if (avail < 0) avail += m_buffSize;
  return avail;
}

#define WAIT { while (ESP.getCycleCount()-start < wait) if (m_intTxEnabled) optimistic_yield(1); wait += m_bitTime; }

size_t OneWireHalfDuplexSoftwareSerial::write(uint8_t b) {
  if (!m_intTxEnabled)
    // Disable interrupts in order to get a clean transmit
    cli();
  unsigned long wait = m_bitTime;
  digitalWrite(communicationPin, HIGH);
  unsigned long start = ESP.getCycleCount();
  // Start bit;
  digitalWrite(communicationPin, LOW);
  WAIT;
  for (int i = 0; i < 8; i++) {
    digitalWrite(communicationPin, (b & 1) ? HIGH : LOW);
    WAIT;
    b >>= 1;
  }
  // Stop bit
  digitalWrite(communicationPin, HIGH);
  WAIT;
  if (!m_intTxEnabled)
    sei();
  return 1;
}

void OneWireHalfDuplexSoftwareSerial::flush() {
  m_inPos = m_outPos = 0;
}

bool OneWireHalfDuplexSoftwareSerial::overflow() {
  bool res = m_overflow;
  m_overflow = false;
  return res;
}

int OneWireHalfDuplexSoftwareSerial::peek() {
  if (!isRxValid() || (m_inPos == m_outPos)) return -1;
  // else
  return m_buffer[m_outPos];
}

void ICACHE_RAM_ATTR OneWireHalfDuplexSoftwareSerial::_rxRead() {
  // Advance the starting point for the samples but compensate for the
  // initial delay which occurs before the interrupt is delivered
  unsigned long wait = m_bitTime + m_bitTime/3 - 500;
  unsigned long start = ESP.getCycleCount();
  uint8_t rec = 0;
  for (int i = 0; i < 8; i++) {
    WAIT;
    rec >>= 1;
    if (digitalRead(communicationPin))
      rec |= 0x80;
  }
  // Stop bit
  WAIT;
  // Store the received value in the buffer unless we have an overflow
  int next = (m_inPos+1) % m_buffSize;
  if (next != m_outPos) {
    m_buffer[m_inPos] = rec;
    m_inPos = next;
  } else {
    m_overflow = true;
  }
  // Must clear this bit in the interrupt register,
  // it gets set even when interrupts are disabled
  GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, 1 << communicationPin);
}

void ICACHE_RAM_ATTR OneWireHalfDuplexSoftwareSerial::rxRead() {
  theInstance->_rxRead();
}

#endif // ARDUINO_ARCH_ESP8266
