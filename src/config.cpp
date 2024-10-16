#include "config.h"

// Hardware parameters (relays)
int relays_pins[4];
byte relaysState;

// Telnet command strings
char str50[50];
char str10[10];
char ipString[16];
char int16String[6];

// Location identity
char idName[50];
char idCode[10];

// Periodic interrupt parameters
volatile int interruptCounter;
hw_timer_t* periodic_timer = nullptr;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

// Message generation parameters
int message_period;
// int rain_lastValue = 0;
// int rain_lastDate;

// General purpose variables declarations
char myMessage[500];
char myMessage_2[500];
char fileName[33];

// Watchdog variables
long uptime;
int reboot_time;
int wifi_connect_timeout;
int ntp_connect_timeout;
int mqtt_connect_timeout;