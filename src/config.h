#ifndef CONFIG_H
#define CONFIG_H

#include <ESP32Time.h>

#include <cstdint>

#include "ESP32Encoder.h"  // PCNT hardware & interrupt ???
#include "driver/pcnt.h"   // ESP32 library for pulse count


// Hardware parameters (relays)
extern int relays_pins[4];
extern byte relaysState;

// Telnet command strings
extern char str50[50];
extern char str10[10];
extern char ipString[16];
extern char int16String[6];

// Identity
extern char idName[50];
extern char idCode[10];

// Periodic interrupt parameters
extern volatile int interruptCounter;
extern hw_timer_t* periodic_timer;
extern portMUX_TYPE timerMux;

// Message generation parameters
extern int message_period;

// General purpose variables declarations
extern char myMessage[500];
extern char myMessage_2[500];
extern char fileName[33];

// Watchdog variables
extern long uptime;
extern int reboot_time;
extern int wifi_connect_timeout;
extern int ntp_connect_timeout;
extern int mqtt_connect_timeout;

#endif