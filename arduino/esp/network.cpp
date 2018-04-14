#include <Arduino.h>

#ifdef ARDUINO_ARCH_ESP32
#include <WiFi.h>
#include <ESPmDNS.h>
#include <esp_wifi.h>
#elif ARDUINO_ARCH_ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#endif

#include "globals.h"
#include "network.h"

unsigned long message_timeout = 10000;
WiFiClient tcp_client;
String receive_buffer;

void set_message_timeout(unsigned long timeout)
{
  message_timeout = timeout;
}


static void confirm_connected(const char* message)
{
  Serial.println("Connected.");
  if (debug_mode) {
    Serial.println(message);
  }
}

void connect(const char* additional_init_params)
{
  unsigned long retry_delay = 1;/*sec*/
  int servername_len = strlen(config.servername);

  while (true) {
    int connected = 0;

#ifdef ARDUINO_ARCH_ESP32
    String _tmp_hostname = "nodexxx";
    _tmp_hostname += config.nodename;
    const char* tmp_hostname = _tmp_hostname.c_str();
    if (servername_len > 6 && strcmp(config.servername + servername_len - 6, ".local") == 0) {
      Serial.printf("Querying hostname '%s' via mDNS...", config.servername);
      char servername_without_suffix[servername_len - 5];
      strncpy(servername_without_suffix, config.servername, servername_len - 6);
      servername_without_suffix[servername_len - 6] = '\0';

      if (!mdns_init()) {
        String _tmp_hostname = "nodexxx";
        if (!mdns_hostname_set(tmp_hostname)) {
          struct ip4_addr addr;
          addr.addr = 0;
          esp_err_t err = mdns_query_a(servername_without_suffix, 5000,  &addr);
          if (!err) {
              IPAddress addr2 = IPAddress(addr.addr);
              Serial.print("Found. Connecting to ");
              Serial.print(addr2);
              Serial.print(':');
              Serial.print(config.port);
              Serial.print("...");

              connected = tcp_client.connect(addr2, config.port);
          } else {
            Serial.println("Failed to obtain server IP address via mDNS.");
          }
        } else {
          Serial.println("Failed to set mDNS hostname.");
        }
        mdns_free();
      } else {
        Serial.println("Failed to start mDNS.");
      }
    } else
#endif
    if (config.servername[0] == '_' && strcmp(config.servername + servername_len - 5, "._tcp") == 0) {
      char servicename[servername_len - 4];
      strncpy(servicename, config.servername, servername_len - 5);
      servicename[servername_len - 5] = '\0';
      Serial.printf("Querying service %s._tcp via mDNS...", servicename);

#ifdef ARDUINO_ARCH_ESP32
      if (MDNS.begin(tmp_hostname)) {
        mdns_result_t * results;
        esp_err_t err = mdns_query_ptr(servicename, "_tcp", 5000, 20,  &results);
        if (err) {
          Serial.println("Error.");
        } else if (!results) {
          Serial.println("Not Found.");
        } else {
          mdns_ip_addr_t * addr = results->addr;
          while(addr){
            if(addr->addr.type == MDNS_IP_PROTOCOL_V4) break;
            addr = addr->next;
          }
          if (addr) {
            IPAddress addr2 = IPAddress(addr->addr.u_addr.ip4.addr);
            Serial.print("Found. Connecting to ");
            Serial.print(addr2);
            Serial.print(':');
            Serial.print(results->port);
            Serial.print("...");
            connected = tcp_client.connect(addr2, results->port);
          } else {
            Serial.println("No IPv4 address attatched to the service.");
          }
          mdns_query_results_free(results);
        }
        MDNS.end();
      }
#elif ARDUINO_ARCH_ESP8266
      if (mdns_started || MDNS.begin(wifi_station_get_hostname())) {
        if (mdns_started) MDNS.update();
        else mdns_started = true;
        if (MDNS.queryService((const char*)(servicename + 1), "tcp") > 0) {
          IPAddress addr = MDNS.IP(0);
          uint16_t port = MDNS.port(0);
          Serial.print("Found. Connecting to ");
          Serial.print(addr);
          Serial.print(':');
          Serial.print(port);
          Serial.print("...");
          connected = tcp_client.connect(addr, port);
        } else {
          Serial.println("Not Found.");
        }
      }
#endif
      else {
        Serial.println("Failed to start mDNS.");
      }
    } else {
      Serial.print("Connecting to ");
      Serial.print(config.servername);
      Serial.print(':');
      Serial.print(config.port);
      Serial.print("...");
      connected = tcp_client.connect(config.servername, config.port);
    }

    if (connected) {
      receive_buffer = "";
      String init_str = "INIT\tnodename:";
      init_str += config.nodename;
      if (additional_init_params) {
        init_str += '\t';
        init_str += additional_init_params;
      }
      send_message(init_str.c_str(), confirm_connected);
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

void disconnect()
{
  tcp_client.stop();
  receive_buffer = "";
}

bool connected()
{
  return tcp_client.connected();
}

bool send_message(const char* message, void (*process_message)(const char*))
{
  tcp_client.write(message, strlen(message));
  tcp_client.write('\n');

  if (debug_mode) {
    Serial.println(message);
  }

  while (true) { // wait for response

    unsigned long t = millis();

    while (receive_buffer.indexOf('\n') < 0) {
      if (millis() > t + message_timeout) {
        Serial.println("Message timeout.");
        return false;
      }
      int available = tcp_client.available();
      if (available) for(int i = 0; i < available; i++) receive_buffer.concat((char)tcp_client.read());
      else delay(100);
    }

    int line_size;
    while ((line_size = receive_buffer.indexOf('\n')) >= 0) {
      char line[line_size + 1];
      strncpy(line, receive_buffer.c_str(), line_size);
      line[line_size] = '\0';
      receive_buffer.remove(0, line_size + 1);
      process_message(line);
    }
    return true;
  }
}
