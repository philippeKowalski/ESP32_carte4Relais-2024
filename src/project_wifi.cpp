#include "project_wifi.h"

// Network parameters
IPAddress ip;
IPAddress dns;
IPAddress gateway;
IPAddress netmask;


// Wifi network parameters
char ssid[20];
char password[20];
WiFiClient wifiClient;

void setup_softAP() {
  // SoftAP to reload SPIFFS
  WiFi.softAPConfig(IPAddress(192, 168, 100, 1), IPAddress(0, 0, 0, 0), IPAddress(255, 255, 255, 0));
  WiFi.softAP("sovOTA", "W1F10VPF");
}

void setup_wifi() {
  // Configuration wifi IP statique
  WiFi.config(ip, gateway, netmask, dns);
  delay(10);

  // We start by connecting to our WiFi network
  Serial.println();
  Serial.printf("Connecting to %s ", ssid);

  WiFi.begin(ssid, password);
  WiFi.setHostname("Carte-4-relais");
  while ((WiFi.status() != WL_CONNECTED) and (wifi_connect_timeout > 0)) {
    delay(500);
    Serial.print(".");
    wifi_connect_timeout--;
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.printf("\nwifi_connect_timeout=%d", wifi_connect_timeout);
  }

  if (wifi_connect_timeout < 1) {
    Serial.println("Wifi connection impossible : rebooting ...\n\n");
    delay(5000);
    ESP.restart();
  }

  // Display wifi connection informations
  Serial.println("\nWiFi connected");
  Serial.print("  IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("  IP gateway: ");
  Serial.println(WiFi.gatewayIP());
  Serial.print("  IP mask: ");
  Serial.println(WiFi.subnetMask());
  Serial.println();
}
