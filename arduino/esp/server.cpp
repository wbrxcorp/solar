#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include "globals.h"
#include "network.h"

static WiFiServer* server;
static WiFiClient client;
static String nodename;

static String receive_buffer;

static const char* pw_str(const char* value)
{
  if (strcmp(value, "0") == 0) return "OFF";
  else if (strcmp(value, "1") == 0) return "ON";
  // else
  return "???";
}

static void process_message(const char* message)
{
  Serial.print("Received: ");
  Serial.println(message);

  if (strlen(message) < 5 || (strncmp(message, "DATA\t", 5) != 0 && strncmp(message, "INIT\t", 5) != 0)) return;
  // else
  const char* pt = message + 5;

  String pv, load, batt, temp, pw, pw1, pw2;

  while (*pt) {
    const char* ptcolon = strchr(pt, ':');
    if (ptcolon == NULL) break; // end parsing string if there's no colon anymore
    char key[ptcolon - pt + 1];
    strncpy(key, pt, ptcolon - pt);
    key[ptcolon - pt] = '\0';
    pt = ptcolon + 1;
    const char* ptdelim = strchr(pt, '\t');
    if (ptdelim == NULL) ptdelim = strchr(pt, '\0');
    // ptdelim can't be NULL here
    char value[ptdelim - pt + 1];
    strncpy(value, pt, ptdelim - pt);
    value[ptdelim - pt] = '\0';
    pt = (*ptdelim != '\0') ? ptdelim + 1 : ptdelim;

    if (strcmp(key, "nodename") == 0) nodename = value;
    else if (strcmp(key, "piw") == 0) pv = value;
    else if (strcmp(key, "load") == 0) load = value;
    else if (strcmp(key, "bv") == 0) batt = value;
    else if (strcmp(key, "temp") == 0) temp = value;
    else if (strcmp(key, "pw") == 0) pw = pw_str(value);
    else if (strcmp(key, "pw1") == 0) pw1 = pw_str(value);
    else if (strcmp(key, "pw2") == 0) pw2 = pw_str(value);
  }

  if (strncmp(message, "DATA\t", 5) == 0) {
    display.clearDisplay();
    display.setCursor(0,0);
    display.print("NODE ");
    display.println(nodename);
    display.print(
      String("PV   ") + pv + "W\n"
        + "LOAD " + load + "W\n"
        + "BATT " + batt + "V\n" +
        + "TEMP " + temp + "deg.\n" +
        + "PW   " + pw + '\n'
        + "PW1  " + pw1 + '\n'
        + "PW2  " + pw2 + '\n');
    display.display();
  }
}

static int receive_message(Stream& inputStream, String& receiveBuffer, void (*process_message)(const char*))
{
  int available = inputStream.available();
  for(int i = 0; i < available; i++) receiveBuffer.concat((char)inputStream.read());
  int line_size;
  int cnt = 0;
  while ((line_size = receiveBuffer.indexOf('\n')) >= 0) {
    char line[line_size + 1];
    strncpy(line, receiveBuffer.c_str(), line_size);
    line[line_size] = '\0';
    receiveBuffer.remove(0, line_size + 1);
    process_message(line);
    cnt++;
  }
  return cnt;
}


// https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266WiFi/examples/WiFiTelnetToSerial/WiFiTelnetToSerial.ino
void setup_server()
{
  server = new WiFiServer(config.port);
  server->begin();
  Serial.println("Server started.");

  size_t servername_len = strlen(config.servername);
  if (servername_len > 6 && config.servername[0] == '_' && strcmp(config.servername + servername_len - 5, "._tcp") == 0) {
    char servicename[servername_len - 5];
    strncpy(servicename, config.servername + 1, servername_len - 6);
    servicename[servername_len - 6] = '\0';
#ifdef ARDUINO_ARCH_ESP8266
    if (start_mdns_if_not_yet()) {
      MDNS.addService((const char*)servicename, "tcp", config.port);
    }
#endif
  }
}

void loop_server()
{
  if (!client) client = server->available();
  if (client && client.connected()) {
    if (receive_message(client, receive_buffer, process_message) > 0) {
      client.write("OK\n");
    }
  }
}
