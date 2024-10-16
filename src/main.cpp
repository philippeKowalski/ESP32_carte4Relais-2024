/* ************************ Dev carte 4 relais par Philippe Kowalski - oct. 2024 ***************************
Objectifs :
0 - Partir du projet ESP_Metreoroi-sensor et l'adapter a la carte 4 relais
1 - connection reseau wifi (IP Statique)
2 - Recuperation heure ntp (serveur OVPF)
3- Watchdog periodique
4- gestion difficultes connection Wifi, synchro NTP
5- gestion absences (réseau, NTP, MQTT, ...)
6- Mise en place des commandes Reboot, syncNTP
7- setRTC()
8- Ajout serveur telnet avec : ls, df, getCgf, getFile, bye et ping
9- Ajout commandes telnet : reboot, setRTC, setNTP, help, infoCfg
10- Ajout telnet > enregistrement config et effaccement fichiers
11- Ajout fonctionnalités OTA - version pas top
12- Ajout fonctions telnet ipCfg, date, time 
13- Demarrage softAP au setup (id=SovOTA/wifiovpf)

Reste à faire :
- commandes telnet : 
      - modification configuration > complique !
- Classement des reponses de la commande "ls"
- possibilite d'utiliser le "*" dans la commande "ls"

****************************************************************************************************/

#include <ArduinoOTA.h>
#include <ESP32Time.h>  // rtc lib
#include <arduino.h>

#include <ctime>

#include "ESP32Encoder.h"  // PCNT hardware & interrupt ???
#include "ESPTelnet.h"
#include "SPIFFS.h"
#include "time.h"

// partie interne au project
#include "config.h"
#include "configuration.h"
#include "json.h"
#include "project_spiffs.h"
#include "project_time.h"
#include "project_wifi.h"
#include "telnet.h"
#include "relais.h"


// --------------------------------------------- Wifi OTA ------------------------------
void setup_OTA() {
  // Port defaults to 3232
  // ArduinoOTA.setPort(3232);

  // Hostname defaults to esp3232-[MAC]
  // ArduinoOTA.setHostname("myesp32");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA
      .onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH)
          type = "sketch";
        else  // U_SPIFFS
          type = "filesystem";

        // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
        SPIFFS.end();
        Serial.println("Start updating " + type);
      })
      .onEnd([]() {
        Serial.println("\nEnd");
      })
      .onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
      })
      .onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR)
          Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR)
          Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR)
          Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR)
          Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR)
          Serial.println("End Failed");
      });

  ArduinoOTA.begin();

  Serial.println("OTA is running");
}


// --------------------------------------------- Periodic timer subs ------------------------------
void IRAM_ATTR onTimer() {
  // Interrupt sub => only set flag
  portENTER_CRITICAL_ISR(&timerMux);
  interruptCounter++;
  portEXIT_CRITICAL_ISR(&timerMux);
}

void setup_periodicTimer() {
  // Init periodic part (prescaler=80 => 1 tick=1 microsecond)
  periodic_timer = timerBegin(1, 80, true);
  timerAttachInterrupt(periodic_timer, &onTimer, true);
  timerAlarmWrite(periodic_timer, 1000000, true);
  timerAlarmEnable(periodic_timer);

  // wait for start of second
  timerStop(periodic_timer);
  Serial.print("\nSynchronizing timer with start of second : ");
  while (rtc.getMillis() > 20) {
    Serial.print(".");
    delay(10);
  }
  // start timer
  timerStart(periodic_timer);
  Serial.println();
}

// --------------------------------------------- Message subs ------------------------------
// void getMessage() {
//   // Written like this to avoid use of String object
//   // sprintf(myMessage, "%04d/%02d/%02d %02d:%02d:%02d %d", rtc.getYear(), rtc.getMonth() + 1, rtc.getDay(), rtc.getHour(true), rtc.getMinute(), rtc.getSecond(), (PulseCounter + offsetCounter));
// }

void setStartingMessage() {
  // getLastMessage(myMessage);
  // getLastValueInMessage(myMessage);

  sprintf(myMessage_2, "%04d/%02d/%02d %02d:%02d:%02d %s", rtc.getYear(), rtc.getMonth() + 1,
          rtc.getDay(), rtc.getHour(true), rtc.getMinute(), rtc.getSecond(), myMessage);

  // offsetCounter = atoi(myMessage);
  // Serial.printf("(fin getMessage) offsetCounter=%d\n", offsetCounter);
  Serial.printf("(fin getMessage)");
}

void printConfiguration() {
  Serial.printf("[identity]\n  name=%s\n  code=%s\n", idName, idCode);
  Serial.printf("[wifi]\n  ip=%s\n  dns=%s\n  gateway=%s\n  netmask=%s\n", ip.toString().c_str(), dns.toString().c_str(), gateway.toString().c_str(), netmask.toString().c_str());
  Serial.printf("[network]\n  Ssid=%s\n  Password=%s\n", ssid, password);
  Serial.printf("[NTP]\n  server=%s\n  server_2=%s\n  server_3=%s\n  ntp_gmtOffset=%d\n  ntp_daylightOffset=%d\n", ntp_server, ntp_server_2, ntp_server_3, ntp_gmtOffset, ntp_daylightOffset);
  // Serial.printf("[messages]\n  message period (sec)=%d\n", message_period);
  Serial.printf("[watchdog]\n  reboot time (sec)=%d\n  wifi_connect_timeout=%d\n  ntp_connect_timeout=%d\n  mqtt_connect_timeout=%d\n  ", reboot_time, wifi_connect_timeout, ntp_connect_timeout, mqtt_connect_timeout);
  Serial.println();
}

void readRelays() {
  // Lecture du fichier "etat des relais"
  File file = SPIFFS.open("/relays_states.txt", "r");
  if (!file || file.isDirectory()) {
    Serial.println("− failed to open file for reading\n");
    return;
  } else {
    // fake pour le moment !!!!!
    // relays_states[0] = 0;
    // relays_states[1] = 0;
    // relays_states[2] = 0;
    // relays_states[3] = 0;
    
    file.close();
  }
}

void writeRelays() {
  File file = SPIFFS.open("/relays_states.txt", "w");
  if (!file || file.isDirectory()) {
    Serial.println("− failed to open file for writing\n");
    return;
  } else {
    // fake pour le moment !!!!!
    // file.write(relays_states[0]);
    // file.write(relays_states[1]);
    // file.write(relays_states[2]);
    // file.write(relays_states[3]);
    
    file.close();
  }
}


// -------------------------- SETUP ---------------------------------
void setup() {
  Serial.begin(115200);
  Serial.printf("Demarrage");

  readLastByteOfFile(SPIFFS, "/relaysState.txt", relaysState);
  setRelays(relaysState);

  strcpy(confFile, "/conf_OVPF.txt");
  Serial.printf("confFile=%s", confFile);
  readConfiguration(confFile);
  printConfiguration();

  setup_wifi();
  setup_ntp();

  // Spiffs actions
  printSpiffsInfo();
  printSpiffsList("/", 2);

  // putMessageToLogFile("Reboot");
  setStartingMessage();
  Serial.printf("  StartingMessage : %s\n", myMessage_2);
  // putMessageToCurrentDayFile(myMessage_2);

  // Starting measurments
  setup_periodicTimer();

  // Starting telnet server
  setup_telnet();

  // Starting softAP
  setup_softAP();

  // Starting OTA
  setup_OTA();

  // Others
  Serial.println("\n------ End of initialisations -------\n");
}

// -------------------------- LOOP ---------------------------------
void loop() {
  telnet.loop();
  ArduinoOTA.handle();

  // InterruptCounter increase every 1 second
  if (interruptCounter > 0) {
    // Flag decrementation
    portENTER_CRITICAL(&timerMux);
    interruptCounter--;
    portEXIT_CRITICAL(&timerMux);

    // Reading, publishing & saving to file every messagePeriod seconds
    if (rtc.getLocalEpoch() % message_period == 0) {
      // getMessage();
      Serial.println("\ncouccou messagePeriod");
      // putMessageToCurrentDayFile(myMessage);
    }

    // watchdog : periodical reboot
    if (uptime >= reboot_time) {
      Serial.println("Uptime > reboot_time ...");
      delay(1000);
      ESP.restart();
    }

    // reboot every day at 00:00
    if ((uptime > 60) && (rtc.getHour() == 0) && (rtc.getMinute() == 0)) {
      Serial.print("It is time to restart ");
      for (int i = 0; i < 5; i++) {
        Serial.print(".");
        delay(1000);
      }
      Serial.print("\n\n");
      ESP.restart();
    }

    Serial.printf(".");
    // delay(2000);

    uptime++;
  }
}
