#ifndef TIME_H
#define TIME_H

#include <ESP32Time.h>

#include <ctime>


// NTP parameters
// const char* ntpServer = "1.pool.ntp.org";
extern char ntp_server[20];
extern char ntp_server_2[20];
extern char ntp_server_3[20];
extern long ntp_gmtOffset;
extern int ntp_daylightOffset;
extern ESP32Time rtc;


// --------------------------------------------- NTP subs ------------------------------
void printNTPLocalTime();
void printRTCLocalTime();
void setup_ntp();
void ntp_sync();
void rtc_setTime(const char* payload, size_t length);

#endif