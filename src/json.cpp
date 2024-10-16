
#include <Arduino.h>
#include <SPIFFS.h>

#define ARDUINOJSON_ENABLE_COMMENTS 1  // Enable comments into Json file
#include <ArduinoJson.h>

//

#include "configuration.h"
#include "json.h"
#include "project_time.h"
#include "project_wifi.h"
#include "telnet.h"

namespace {
  char jsonString[2000];
  StaticJsonDocument<2000> jsonConfig;
}  // namespace

// --------------------------------------------- Configuration subs ------------------------------
IPAddress parseIpAddress(const JsonString& input) {
  uint8_t x1, x2, x3, x4;
  sscanf(input.c_str(), "%d.%d.%d.%d", &x1, &x2, &x3, &x4);
  auto result = IPAddress(x1, x2, x3, x4);
  return result;
}

bool isValidInteger(String str) {
  // Check this string is an integer
  if (str.length() == 0 ) return false;

  // Check that all chars are numbers
  for (int i = 0; i < str.length(); i++) {
    if (!isDigit(str[i])) return false;
  }
  return true;
}

bool isValidOctet(String str) {
  // Check this part is an integer between 0 et 255
  if (str.length() == 0 || str.length() > 3) return false;

  // If this number starts by 0 but is not 0, it is not valid
  if (str[0] == '0' && str.length() > 1) return false;
    
  // Check that all chars are numbers
  for (int i = 0; i < str.length(); i++) {
    if (!isDigit(str[i])) return false;
  }
    
  // Convert the input string in an integer
  int num = str.toInt();
   // Check that this integer is between 0 and 255
  return num >= 0 && num <= 255;
}

bool isValidIP(String ip) {
  int nombrePoints = 0;
  String octet = "";
  int len = ip.length();

  for (int i=0; i<len; i++) {
    char c = ip[i];

    if (c == '.') {
      nombrePoints++;
      if (!isValidOctet(octet)) {
        return false;
      }
      
      octet = "";
    } else {
      octet += c;
    }
  }
 
  // Vérifie le dernier octet après la dernière itération
  if (!isValidOctet(octet)) {
    return false;
  }

  // Il doit y avoir exactement 3 points dans une adresse IP valide
  return nombrePoints == 3;
}

void harvestconfiguration(){
  // Set up to date of new jsonDocument with current configuration parameters
    // 0 - identity params
  jsonConfig["identity"]["name"] = idName;
  jsonConfig["identity"]["code"] = idCode;

  // 1 - Wifi & network params
  //strcpy(ssid, jsonConfig["wifi"]["ssid"].as<JsonString>().c_str());
  jsonConfig["wifi"]["ssid"] = ssid;
  jsonConfig["wifi"]["pwd"] = password;
  jsonConfig["network"]["ip"] = ip;
  jsonConfig["network"]["dns"] = dns;
  jsonConfig["network"]["gateway"] = gateway;
  jsonConfig["network"]["netmask"] = netmask;

  // 2 - NTP params
  jsonConfig["NTP"]["server"] = ntp_server;
  jsonConfig["NTP"]["server_2"] = ntp_server_2;
  jsonConfig["NTP"]["server_3"] = ntp_server_3;

  jsonConfig["NTP"]["gmtOffset"] = ntp_gmtOffset;
  jsonConfig["NTP"]["daylightOffset"] = ntp_daylightOffset;

  // // 3 - Watchdog params
  jsonConfig["watchdog"]["reboot_time"] = reboot_time;
  jsonConfig["watchdog"]["wifi_connect_timeout"] = wifi_connect_timeout;
  jsonConfig["watchdog"]["ntp_connect_timeout"] = ntp_connect_timeout;
  jsonConfig["watchdog"]["mqtt_connect_timeout"] = mqtt_connect_timeout;

  // // 4 - Messages params
  jsonConfig["publication"]["period"] = message_period;
}

void readConfiguration(const char* fichConf) {
  if (!SPIFFS.begin()) {
    Serial.println("Failed to mount SPIFFS");

    // Setup softAP for backup possibility
    setup_softAP();
    setup_telnet();

    // Waiting a long time before reboot = 10 mn
    delay(600000);
    ESP.restart();
  } else {
    File file = SPIFFS.open(fichConf, FILE_READ);

    if (!file) {
      Serial.println("There was an error opening the file");
      return;
    } else {
      Serial.println("\nLoading config file\n");

      //StaticJsonDocument<2000> doc;
      DeserializationError error = deserializeJson(jsonConfig, file);

      if (error) {
        Serial.println("Json deserialization error...");
      } else {
        // 0 - identity params
        strcpy(idName, jsonConfig["identity"]["name"]);
        strcpy(idCode, jsonConfig["identity"]["code"]);

        // 1 - Wifi & network params
        //strcpy(ssid, jsonConfig["wifi"]["ssid"].as<JsonString>().c_str());
        strcpy(ssid, jsonConfig["wifi"]["ssid"]);
        strcpy(password, jsonConfig["wifi"]["pwd"]);
        ip = parseIpAddress(jsonConfig["network"]["ip"].as<JsonString>());
        dns = parseIpAddress(jsonConfig["network"]["dns"].as<JsonString>());
        gateway = parseIpAddress(jsonConfig["network"]["gateway"].as<JsonString>());
        netmask = parseIpAddress(jsonConfig["network"]["netmask"].as<JsonString>());

        // 2 - NTP params
        strcpy(ntp_server, jsonConfig["NTP"]["server"].as<JsonString>().c_str());
        strcpy(ntp_server_2, jsonConfig["NTP"]["server_2"].as<JsonString>().c_str());
        strcpy(ntp_server_3, jsonConfig["NTP"]["server_3"].as<JsonString>().c_str());
        long x2y1 = jsonConfig["NTP"]["gmtOffset"];
        ntp_gmtOffset = x2y1;
        int x2y2 = jsonConfig["NTP"]["daylightOffset"];
        ntp_daylightOffset = x2y2;

        // 3 - Watchdog params
        int x5y1 = jsonConfig["watchdog"]["reboot_time"];
        reboot_time = x5y1;
        int x5y2 = jsonConfig["watchdog"]["wifi_connect_timeout"];
        wifi_connect_timeout = x5y2;
        int x5y3 = jsonConfig["watchdog"]["ntp_connect_timeout"];
        ntp_connect_timeout = x5y3;
        int x5y4 = jsonConfig["watchdog"]["mqtt_connect_timeout"];
        mqtt_connect_timeout = x5y4;

        // 4 - Messages params
        int x4y1 = jsonConfig["publication"]["period"];
        message_period = x4y1;
      }
    }

    file.close();

    harvestconfiguration();
  }
}

void saveConfiguration(const char* fileName) {
  // Write jsonDocument named jsonConfig into file named fileName 

  // Put actual values of configuration in jsonDocument
  harvestconfiguration();

  // write jsonDocument to file
  File file = SPIFFS.open(fileName, "w");
  if (!file) {
    Serial.printf("\n File system is not OK");
  } else {
    serializeJsonPretty(jsonConfig, jsonString);
    file.println(jsonString);
    file.close();
    Serial.printf(" Configuration saved to %s\r\n", fileName);
    Serial.println(jsonString);
  }
}