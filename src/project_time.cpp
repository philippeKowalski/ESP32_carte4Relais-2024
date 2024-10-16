#include "project_time.h"
#include "project_wifi.h"
#include "config.h"

char ntp_server[20];
char ntp_server_2[20];
char ntp_server_3[20];
long ntp_gmtOffset;
int ntp_daylightOffset;
ESP32Time rtc;

// --------------------------------------------- NTP subs ------------------------------
void printNTPLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.print("Get NTP time : ");
  Serial.println(&timeinfo, "%Y/%m/%d %H:%M:%S");
}

void printRTCLocalTime() {
  Serial.println(rtc.getTime("Get RTC time : %Y/%m/%d %H:%M:%S"));
}

void setup_ntp() {
  struct tm timeinfo;
  configTime(ntp_gmtOffset, ntp_daylightOffset, ntp_server, ntp_server_2, ntp_server_3);
  Serial.printf("\nNTP synchronization ");
  while (!getLocalTime(&timeinfo) && (ntp_connect_timeout > 0)) {
    Serial.print(".");
    delay(100);
    ntp_connect_timeout--;
  }

  if (rtc.getYear() != 1970) {
    Serial.print("\nRTC synchronized\n  Time set to ");
  } else {
    Serial.print("\nSynchro NTP impossible\n  Time set to ");
  }
  Serial.println(&timeinfo, "%Y/%m/%d %H:%M:%S\n");
}

void ntp_sync() {
  struct tm timeinfo;
  configTime(ntp_gmtOffset, ntp_daylightOffset, ntp_server, ntp_server_2, ntp_server_3);
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Synchro NTP impossible");
  } else {
    delay(1000);
    sprintf(myMessage_2, "Time (NTP) set to %04d/%02d/%02d %02d:%02d:%02d", rtc.getYear(), rtc.getMonth() + 1, rtc.getDay(), rtc.getHour(), rtc.getMinute(), rtc.getSecond());
    Serial.println(myMessage_2);
  }
}

void rtc_setTime(const char* payload, size_t length) {
  // building number by conversion char/int & computation
  int tm[6] = {0, 0, 0, 0, 0, 0};
  int i;
  Serial.printf("length=%d\n", length);
  if (length == 26) {
    for (i = 0; i < 2; i++) tm[0] = 10 * tm[0] + (int)payload[24 + i] - 48;
    for (i = 0; i < 2; i++) tm[1] = 10 * tm[1] + (int)payload[21 + i] - 48;
    for (i = 0; i < 2; i++) tm[2] = 10 * tm[2] + (int)payload[18 + i] - 48;
    for (i = 0; i < 2; i++) tm[3] = 10 * tm[3] + (int)payload[15 + i] - 48;
    for (i = 0; i < 2; i++) tm[4] = 10 * tm[4] + (int)payload[12 + i] - 48;
    for (i = 0; i < 4; i++) tm[5] = 10 * tm[5] + (int)payload[7 + i] - 48;
    Serial.printf("rtc.setTime(%02d,%02d,%02d,%02d,%02d,%04d)\n", tm[0], tm[1], tm[2], tm[3], tm[4], tm[5]);
    rtc.setTime(tm[0], tm[1], tm[2], tm[3], tm[4], tm[5]);
  } else
    Serial.println("Syntax error - format is 'setRTC yyyy,MM,dd,hh,mm,ss'");
}
