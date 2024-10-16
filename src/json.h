#ifndef JSON_H
#define JSON_H

#include <WiFi.h>

bool isValidInteger(String str);
bool isValidOctet(String str);
bool isValidIP(String ip);

void readConfiguration(const char* fichConf);
void saveConfiguration(const char* fileName);

void arrangeJson();

#endif