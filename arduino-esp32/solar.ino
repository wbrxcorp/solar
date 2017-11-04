// arduino --upload --board espressif:esp32:esp32doit-devkit-v1:FlashFreq=80,UploadSpeed=115200 --port /dev/ttyUSB0 solar.ino
HardwareSerial RS485(2);  // RX=16,TX=17

const char string_0[] PROGMEM = "Hello, World!";

void setup() {
  Serial.begin(115200);
}

void loop() {
  Serial.println(string_0);
  delay(1000);
}
