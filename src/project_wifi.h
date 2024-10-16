#ifndef WIFI_H
#define WIFI_H


#include <Arduino.h>
#include <IPAddress.h>
#include <WiFi.h>
#include <WiFiClient.h>

//

#include "config.h"

// Network parameters
extern IPAddress ip;
extern IPAddress dns;
extern IPAddress gateway;
extern IPAddress netmask;


// Wifi network parameters
extern char ssid[];
extern char password[];
extern WiFiClient wifiClient;

// --------------------------------------------- Wifi Subs ------------------------------
void setup_softAP();

void setup_wifi();

#endif