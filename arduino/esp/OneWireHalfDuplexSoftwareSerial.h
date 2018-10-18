/*
OneWireHalfDuplexSoftwareSerial.h - espsoftwareserial modified for limited purpose

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

#ifndef OneWireHalfDuplexSoftwareSerial_h
#define OneWireHalfDuplexSoftwareSerial_h

#include <inttypes.h>
#include <Stream.h>

class OneWireHalfDuplexSoftwareSerial : public Stream {
public:
  static OneWireHalfDuplexSoftwareSerial* getInstance(unsigned int buffSize = 64);

  void begin(long speed);
  long baudRate();

  bool overflow();
  int peek();

  virtual size_t write(uint8_t byte);
  virtual int read();
  virtual int available();
  virtual void flush();
  operator bool() {return isRxValid();}

  // Disable or enable interrupts on the rx pin
  void enableRx(bool on);
  // One wire control
  void enableTx(bool on);

  static void rxRead();

  inline bool isRxValid() { return m_buffer != NULL; }

  using Print::write;

protected:
  OneWireHalfDuplexSoftwareSerial(unsigned int buffSize = 64);
  virtual ~OneWireHalfDuplexSoftwareSerial();
  void _rxRead();

private:
  static OneWireHalfDuplexSoftwareSerial* theInstance; // singleton instance

  // Member variables
  const int COMM_PIN = 0;
  bool m_rxEnabled;
  bool m_overflow;
  unsigned long m_bitTime;
  volatile unsigned int m_inPos, m_outPos;
  int m_buffSize;
  uint8_t *m_buffer;
};

#endif // OneWireHalfDuplexSoftwareSerial_h
