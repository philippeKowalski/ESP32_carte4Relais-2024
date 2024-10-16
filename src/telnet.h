#ifndef TELNET_H
#define TELNET_H

#include <ESPTelnet.h>

#include <cstdint>

// telnet variables
extern char bufferTelnet[];
extern char bufferFile[];
extern int indexBufTelnet;
extern int nbMessageTelnet;
extern ESPTelnet telnet;
extern uint16_t telnet_port;

void setup_telnet();
void printSpiffsListToTelnet(const char* dirname, uint8_t levels);

#endif