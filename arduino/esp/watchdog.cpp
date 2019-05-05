#ifdef ARDUINO_ARCH_ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266Ping.h>
#elif defined ARDUINO_ARCH_ESP32
#include <WiFi.h>
#include <esp_wifi.h>

class _Ping {
public:
  bool ping(const char* host, int count=5) { Serial.println("Not implemented yet"); return false; }
};

_Ping Ping;
#endif

#include "globals.h"
#include "watchdog.h"

unsigned long last_watch = 0L;
unsigned long last_print_message = 0L;
uint16_t first_watch_sec = 300;
uint16_t watch_interval_sec = 3600;
uint16_t print_message_interval_sec = 5;

int relay_pin = 13;

void setup_watchdog()
{
  Serial.println("Setup Watchdog");
  pinMode(relay_pin, OUTPUT);
  digitalWrite(relay_pin, LOW);

  last_watch = millis() - (watch_interval_sec - first_watch_sec) * 1000L;
}

static bool ping(const char* host)
{
  Serial.print("Pinging ");
  Serial.print(host);
  Serial.print(" ...");
  bool rst = Ping.ping(host, 5);
  Serial.println(rst? "OK" : "NG");
  return rst;
}

static void bite()
{
  digitalWrite(relay_pin, HIGH);
  delay(3000);
  digitalWrite(relay_pin, LOW);
}

static void watch()
{
  Serial.print("Connecting to WiFi AP...");
#ifdef ARDUINO_ARCH_ESP8266
  WiFi.forceSleepWake();
#endif
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.begin(config.ssid, config.key);
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (millis() - startTime > 60000/* 1minute */) {
      Serial.println("WiFi connection timeout. Bow wow!");
      bite();
      return;
    }
  }
  Serial.println("Connected.");

  if (!ping("8.8.8.8") && !ping("8.8.4.4")) {
    Serial.println("No ping reply. Bow wow!");
    bite();
  }
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
}

void loop_watchdog()
{
  unsigned long now = millis();
  if ((Serial.available() && Serial.read() == '\r') || now > last_watch + watch_interval_sec * 1000L) {
    watch();
    last_watch = now;
  }
  if (now > last_print_message + print_message_interval_sec * 1000L) {
    Serial.println("Press enter to check connectivity manually");
    last_print_message = now;
  }
  delay(100);
}
