#include "telnet.h"

#include <Arduino.h>
#include <SPIFFS.h>

//

// #include "config.h"
#include "configuration.h"
#include "config.h"
#include "json.h"
#include "project_spiffs.h"
#include "project_wifi.h"
#include "project_time.h"

char bufferTelnet[500];
char bufferFile[500];
int indexBufTelnet = 0;
ESPTelnet telnet;
uint16_t telnet_port = 22;

void errorMsg(String error, bool restart = true) {
  Serial.println(error);
  if (restart) {
    Serial.println("Rebooting now...");
    delay(2000);
    ESP.restart();
    delay(2000);
  }
}

bool charequal(const char* rhs, const char* lhs, size_t length) {
  // length is length we want to compare
  for (size_t i = 0; i < length; i++) {
    // tab sizes not equal
    if (lhs[i] == 0) {
      return false;
    }
    // Content not equal
    if (rhs[i] != lhs[i]) {
      return false;
    }
  }
  return true;
}

IPAddress parseIpAddress(const String input) {
  uint8_t x1, x2, x3, x4;
  sscanf(input.c_str(), "%d.%d.%d.%d", &x1, &x2, &x3, &x4);
  auto result = IPAddress(x1, x2, x3, x4);
  return result;
}

void stprint(String message){
  Serial.print(message);
  telnet.print(message);
}

void stprintln(String message){
  Serial.println(message);
  telnet.println(message);
}

void fileNameCorrection(char* goodName, const char* badName) {
  // Add '/' before file name if necessary
  char msg[50];
  int decal = 0;
  int i = 0;

  if (badName[0] != '/') {
    goodName[0] = '/';
    decal = 1;
  }
  while (badName[i] != '\0') {
    goodName[i + decal] = badName[i];
    i++;
  }
  goodName[i + decal] = '\0';

  sprintf(msg, "%s corriges en %s", badName, goodName);
  stprintln(msg);
}

void prompt() {
  telnet.print(idCode);
  telnet.print(">");
}

void onTelnetConnect(String ip) {
  Serial.printf("- Telnet: ");
  Serial.print(ip);
  Serial.println(" connected");
  telnet.println("\nWelcome " + telnet.getIP());
  telnet.println("(Use 'bye'  to disconnect.)");
  telnet.println("========== type ? to get complete command set ===========");
  prompt();
  indexBufTelnet = 0;
}

void onTelnetDisconnect(String ip) {
  Serial.print("- Telnet: ");
  Serial.print(ip);
  Serial.println(" disconnected");
  telnet.stop();
  if (telnet.begin(telnet_port, false)) {
    Serial.println("running");
  } else {
    Serial.println("error.");
    errorMsg("Will reboot...");
  }
}

void onTelnetReconnect(String ip) {
  Serial.print("- Telnet: ");
  Serial.print(ip);
  Serial.println(" reconnected");
  prompt();
}

void onTelnetConnectionAttempt(String ip) {
  Serial.print("- Telnet: ");
  Serial.print(ip);
  Serial.println(" tried to connected");
}

void printFileToTelnet(const char* path) {
  int idx = 0;
  Serial.printf("Sending file to telnet : %s**\r\n", path);

  File file = SPIFFS.open(path);
  if (!file || file.isDirectory()) {
    stprintln("− failed to open file for reading\n");
    return;
  }

  Serial.println("− read from file:");
  while (file.available()) {
    bufferFile[idx] = file.read();
    if ((bufferFile[idx] == 10) && (bufferFile[idx - 1] == 13)) {  // CRLF (Carriage Return & Line Feed)
      bufferFile[idx + 1] = '\0';
      idx = 0;
      stprint(bufferFile);
    } else {
      idx++;
    }
  }
  file.close();
}

void printDfToTelnet() {
  char tmsg[100];
  int freeSpace = SPIFFS.totalBytes() - SPIFFS.usedBytes();
  double pctUsed = 100 * double(SPIFFS.usedBytes()) / SPIFFS.totalBytes();

  sprintf(tmsg, "SPIFFS %*d %*d %*d %*.0f", 15, SPIFFS.totalBytes(), 8, freeSpace, 8, SPIFFS.usedBytes(), 8, pctUsed);
  telnet.println("\n\n===== File system info =====");
  telnet.println("File system      Total     Free     Used      Use\%");
  telnet.print(tmsg);
  telnet.println("%");
}

void onInput(String str) {
  char tab[50];
  char myMsg[80];

  str.toCharArray(tab, str.length() + 1);
  int i = 0;
  while ((tab[i] != 0) && (i < 50)) {
    bufferTelnet[indexBufTelnet] = tab[i];
    indexBufTelnet++;
    i++;
  }

  if ((bufferTelnet[indexBufTelnet - 2] == 13) && (bufferTelnet[indexBufTelnet - 1] == 10)) {
    bufferTelnet[indexBufTelnet] = '\0';
    indexBufTelnet = 0;
    Serial.printf("\nMessage : %s\n", bufferTelnet);

    // Look for key words
    if (charequal(bufferTelnet, "ping", 4)) {
      stprintln("Response of : ping received !");
    } else if (charequal(bufferTelnet, "bye", 3)) {
      stprintln("telnet deconnection ...");
      telnet.disconnectClient();
    } else if (charequal(bufferTelnet, "exit", 4)) {
      // disconnection
      stprintln("telnet deconnection ...");
      telnet.disconnectClient();
    } else if (charequal(bufferTelnet, "getCfg", 6)) {
      // Print configuration
      sprintf(myMsg, "telnet sending config file %s\r\n=== Sending config file ",confFile);
      // Serial.print("telnet sending config file ");
      // Serial.println(confFile);
      // telnet.print("=== Sending config file ");
      // telnet.print(confFile);
      // telnet.println(" ===");
      // printFileToTelnet("/conf.txt");
      stprint(myMsg);
      printFileToTelnet(confFile);
    } else if (charequal(bufferTelnet, "df", 2)) {
      // Print FS infos
      printDfToTelnet();
    } else if (charequal(bufferTelnet, "ls", 2)) {
      // list '/'
      Serial.println("telnet sending SPIFFS/ content");
      // printSpiffsListToTelnet("/", 2);
      getLsSorted(myMsg);
      telnet.println(myMsg); 

      getSpiffsLs(myMsg, "/");   
      telnet.println("test de getSpiffsLs : ");
      telnet.print(myMsg); 
    } else if (charequal(bufferTelnet, "lss", 3)) {
      // list '/' starting by motif
      Serial.println("telnet sending SPIFFS/ content");
      // printSpiffsListToTelnet("/", 2);
      getLsSorted(myMsg);
      telnet.print(myMsg);      
    } else if (charequal(bufferTelnet, "date", 4)) {
      sprintf(myMessage, "%04d/%02d/%02d %02d:%02d:%02d", rtc.getYear(), rtc.getMonth() + 1, rtc.getDay(), rtc.getHour(true), rtc.getMinute(), rtc.getSecond());
      telnet.println(myMessage);
    } else if (charequal(bufferTelnet, "time", 4)) {
      sprintf(myMessage, "%02d:%02d:%02d", rtc.getHour(true), rtc.getMinute(), rtc.getSecond());
      telnet.println(myMessage);
    } else if (charequal(bufferTelnet, "setNTP", 6)) {
      // Request an NTP synchro
      stprintln("NTP Synchro requested ");
      ntp_sync();
    } else if (charequal(bufferTelnet, "reboot", 6)) {
      // Reboot
      stprintln("Rebooting ... ");
      delay(500);
      stprintln("Bye");
      delay(3000);
      ESP.restart();
    } else if (charequal(bufferTelnet, "help", 4) or charequal(bufferTelnet, "?", 1)) {
      // Display help
      printFileToTelnet("/help.txt");
    } else if (charequal(bufferTelnet, "infoCfg", 7)) {
      // Display meaning of configuration parameters
      printFileToTelnet("/infoConfiguration.txt");
    } else if (charequal(bufferTelnet, "setRTC", 6)) {
      // set RTC date and time
      int i = 7;
      while (bufferTelnet[i] != '\0') {
        i++;
      }
      bufferTelnet[i - 2] = '\0';  // Suppr. CRLF

      stprintln("SetRTC requested");
      // telnet.println("Set RTC requested");

      rtc_setTime(bufferTelnet, i - 2);
    } else if (charequal(bufferTelnet, "more", 4)) {
      // Display file content
      char command[50];
      char goodFileName[50];
      int i = 0;
      
      // recup file name
      while (bufferTelnet[i + 5] != '\0') {
        command[i] = bufferTelnet[i + 5];
        i++;
      }
      command[i - 2] = '\0';  // Suppress CR LF

      fileNameCorrection(goodFileName, command);
      sprintf(myMsg, "===> Sending file %s\r\n", goodFileName);
      stprint(myMsg);
      printFileToTelnet(goodFileName);

    // Configuration modification using 'set'    
    } else if (charequal(bufferTelnet, "set", 3)) {
      // Modify value of idCode = identity/code
      char paramName[10];
      char paramValue[50];
      int i = 0;

      while (bufferTelnet[i + 4] != ' ') {
        paramName[i] = bufferTelnet[i + 4];
        i++;
      }
      paramName[i] = '\0';
      i++;

      int j = 0;
      while (bufferTelnet[i + 4] != '\0') {
        paramValue[j] = bufferTelnet[i + 4];
        j++;
        i++;
      }
      paramValue[j - 2] = '\0';  // Suppress CR LF
      
      sprintf(myMsg, "===> Setting %s to %s\r\n", paramName, paramValue);
      if (charequal(paramName, "idCode", 6)) {
        strcpy(idCode, paramValue);
        stprint(myMsg);
      } else if (charequal(paramName, "idName", 6)) {
        strcpy(idName, paramValue);
        stprint(myMsg);
      } else if (charequal(paramName, "ssid", 4)) {
        strcpy(ssid, paramValue);
        stprint(myMsg);
      } else if (charequal(paramName, "password", 8)) {
        strcpy(password, paramValue);
        stprint(myMsg);
      } else if (charequal(paramName, "ip", 2)) {
        strcpy(ipString, paramValue);
        if (!(isValidIP(ipString))) {
          sprintf(myMsg, "WARNING %s is not a valid ip address\r\n", ipString);
        } else {
          // affectation ip
          ip = parseIpAddress(ipString);
        }
        stprint(myMsg);
      } else if (charequal(paramName, "gateway", 7)) {
        strcpy(ipString, paramValue);
        if (!(isValidIP(ipString))) {
          sprintf(myMsg, "WARNING %s is not a valid ip address\r\n", ipString);
        } else {
          // affectation ip
          gateway = parseIpAddress(ipString);
        }
        stprint(myMsg);
      } else if (charequal(paramName, "netmask", 7)) {
        strcpy(ipString, paramValue);
        if (!(isValidIP(ipString))) {
          sprintf(myMsg, "WARNING %s is not a valid net mask\r\n", ipString);
        } else {
          // affectation ip
          netmask = parseIpAddress(ipString);
        }
        stprint(myMsg);
      } else if (charequal(paramName, "ntpServer", 9)) {
        strcpy(ipString, paramValue);
        if (!(isValidIP(ipString))) {
          sprintf(myMsg, "WARNING %s is not a valid ip address\r\n", ipString);
        } else {
          strcpy(ntp_server, ipString);
        }
        stprint(myMsg);
      } else if (charequal(paramName, "rebootPeriod", 12)) {
        if (isValidInteger(paramValue)) {
          reboot_time = atoi(paramValue);
          Serial.println(reboot_time);
        } else {
          sprintf(myMsg, "WARNING %s is not a valid integer !\r\n", paramValue);
        }
        stprint(myMsg);
        sprintf(myMsg, "valeur reboot_time = %d", reboot_time);
        stprint(myMsg);
      }
    } else if (charequal(bufferTelnet, "rm", 2)) {
      // Display file content
      char command[50];
      char goodFileName[50];
      int i = 0;

      // Recuperation file name
      while (bufferTelnet[i + 3] != '\0') {
        command[i] = bufferTelnet[i + 3];
        i++;
      }
      command[i - 2] = '\0';  // Suppress CR LF

      fileNameCorrection(goodFileName, command);
      sprintf(myMsg, "===> Erasing file %s\r\n", goodFileName);
      stprint(myMsg);
      rmFile(goodFileName);
    } else if (charequal(bufferTelnet, "mv", 2)) {
      char source[20];
      char dest[20];
      char goodSource[21];
      char goodDest[21];
      int i = 0;
      int j = 0;

      while ((bufferTelnet[i + 3] != '\0') and (bufferTelnet[i + 3] != ' ')) {
        source[i] = bufferTelnet[i + 3];
        i++;
      }
      if (bufferTelnet[i + 3] == ' ') {
        source[i] = '\0';
        i++;
        j = i;
        while (bufferTelnet[i + 3] != '\0') {
          dest[i - j] = bufferTelnet[i + 3];
          i++;
        }
      } else {
        dest[0] = '\0';
      }

      dest[i - j - 2] = '\0';  // Suppress CR LF

      fileNameCorrection(goodDest, dest);
      fileNameCorrection(goodSource, source);
      sprintf(myMsg, "===> Moving file *%s* to *%s*\r\n", goodSource, goodDest);
      stprint(myMsg);
      mvFile(goodSource, goodDest);

    } else if (charequal(bufferTelnet, "ipCfg", 5) or charequal(bufferTelnet, "ip", 2)) {
      // To write !!!
      telnet.print("    ip :");
      telnet.println(ip);
      telnet.print("    gw :");
      telnet.println(gateway);
      telnet.print("netmask :");
      telnet.println(netmask);
    } else if (charequal(bufferTelnet, "saveCfg", 7)) {
      // Display file content
      char command[50];
      char goodFileName[51];
      int i = 0;
      while (bufferTelnet[i + 8] != '\0') {
        command[i] = bufferTelnet[i + 8];
        i++;
      }
      command[i - 2] = '\0';  // Suppress CR LF

      fileNameCorrection(goodFileName, command);
      sprintf(myMsg, "===> Saving configuration to %s\r\n", goodFileName);
      stprint(myMsg);

      saveConfiguration(goodFileName);
    } else {
      telnet.println("Unknow command");
    }

    prompt();
  }
}

void setup_telnet() {
  // passing on functions for various telnet events
  telnet.onConnect(onTelnetConnect);
  telnet.onConnectionAttempt(onTelnetConnectionAttempt);
  telnet.onReconnect(onTelnetReconnect);
  telnet.onDisconnect(onTelnetDisconnect);
  telnet.onInputReceived(onInput);
  telnet.setLineMode(false);

  Serial.print("- Telnet: ");
  if (telnet.begin(telnet_port, false)) {
    Serial.println("running");
  } else {
    Serial.println("error.");
    errorMsg("Will reboot...");
  }
}
