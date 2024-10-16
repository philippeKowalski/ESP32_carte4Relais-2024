#include <Arduino.h>
#include <SPIFFS.h>

#include "config.h"
#include "project_spiffs.h"
#include "project_time.h"

void rmFile(const char* fileName) {
  if (SPIFFS.exists(fileName)) {
    SPIFFS.remove(fileName);
    Serial.printf("  File %s removed\r\n", fileName);
  } else {
    Serial.printf("  File %s not found\r\n", fileName);
  }
}

void printSpiffsInfo() {
  unsigned int totalBytes = SPIFFS.totalBytes();
  unsigned int usedBytes = SPIFFS.usedBytes();
  char myMsg[50];

  sprintf(myMsg, "Total space:      %d bytes", SPIFFS.totalBytes());
  Serial.println(myMsg);
  sprintf(myMsg, "Total space used: %d bytes", SPIFFS.usedBytes());
  Serial.println(myMsg);
  sprintf(myMsg, "Total free space: %d bytes", SPIFFS.totalBytes() - SPIFFS.usedBytes());
  Serial.println(myMsg);
}

void printDf() {
  char myMsg[100];
  int freeSpace = SPIFFS.totalBytes() - SPIFFS.usedBytes();
  double pctUsed = 100 * double(SPIFFS.usedBytes()) / SPIFFS.totalBytes();

  sprintf(myMsg, "SPIFFS %*d %*d %*d %*.0f", 15, SPIFFS.totalBytes(), 8, freeSpace, 8, SPIFFS.usedBytes(), 8, pctUsed);
  Serial.println("\n\n===== File system info =====");
  Serial.println("File system      Total     Free     Used      Use\%");
  Serial.print(myMsg);
  Serial.println("%");
}

void getSpiffsLs(char* listDir, const char* dirname) {
  File root = SPIFFS.open("/");   // Ouvrir la racine du système de fichiers
  File file = root.openNextFile(); // Ouvrir le premier fichier
  
  // Création d'une liste dynamique pour stocker les noms de fichiers
  String files[100];  // Suppose un maximum de 100 fichiers
  int fileCount = 0;
  
  // Parcourir tous les fichiers et stocker leurs noms
  while (file) {
    files[fileCount] = String(file.name());  // Ajouter le nom du fichier dans le tableau
    fileCount++;
    file = root.openNextFile();  // Ouvrir le fichier suivant
  }
  
  // Afficher les fichiers triés
  char resultat[80];
  resultat[0] = '\0';
  listDir[0] = '\0';

  for (int i = 0; i < fileCount; i++) {
    files[i].toCharArray(resultat, 25);
    strcat(listDir, resultat);
    strcat(listDir, "\r\n");
  }
}

void getLsSorted(char* listDir) {
  File root = SPIFFS.open("/");   // Ouvrir la racine du système de fichiers
  File file = root.openNextFile(); // Ouvrir le premier fichier
  
  // Création d'une liste dynamique pour stocker les noms de fichiers
  String files[100];  // Suppose un maximum de 100 fichiers
  int fileCount = 0;
  
  // Parcourir tous les fichiers et stocker leurs noms
  while (file) {
    files[fileCount] = String(file.name());  // Ajouter le nom du fichier dans le tableau
    fileCount++;
    file = root.openNextFile();  // Ouvrir le fichier suivant
  }
  
  // Tri des fichiers par ordre alphabétique (tri à bulles pour simplicité)
  for (int i = 0; i < fileCount - 1; i++) {
    for (int j = i + 1; j < fileCount; j++) {
      if (files[i] > files[j]) {
        String temp = files[i];
        files[i] = files[j];
        files[j] = temp;
      }
    }
  }
  
  // Afficher les fichiers triés
  char resultat[80];
  resultat[0] = '\0';
  listDir[0] = '\0';

  for (int i = 0; i < fileCount; i++) {
    files[i].toCharArray(resultat, 25);
    strcat(listDir, resultat);
    strcat(listDir, "\r\n");
  }
}

void printSpiffsList(const char* dirname, uint8_t levels) {
  Serial.printf("Listing directory: %s\r\n", dirname);

  File root = SPIFFS.open(dirname);
  if (!root) {
    Serial.println(" failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println(" not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels) {
        printSpiffsList(file.name(), levels - 1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("\tSIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
  Serial.println();
}

bool enoughFreeSpaceOnSpiffs(const int limite) {
  unsigned int totalBytes = SPIFFS.totalBytes();
  unsigned int usedBytes = SPIFFS.usedBytes();
  if ((totalBytes - usedBytes) < limite) {
    return false;
  } else {
    return true;
  }
}

void readFile(const char* path) {
  Serial.printf("Reading file: %s\r\n", path);

  File file = SPIFFS.open(path);
  if (!file || file.isDirectory()) {
    Serial.println("− failed to open file for reading\n");
    return;
  }

  Serial.println("− read from file:");
  while (file.available()) {
    Serial.write(file.read());
  }
  file.close();
}

void printSpiffsFileContent(const char* fileName) {
  readFile(fileName);
}

void mvFile(const char* source, const char* dest) {
  if (SPIFFS.exists(source)) {
    if (SPIFFS.exists(dest)) {
      Serial.printf("  File %s already exists\r\n", dest);
    } else {
      SPIFFS.rename(source, dest);
      Serial.printf("  File %s moved to %s\r\n", source, dest);
    }
  } else {
    Serial.printf("  File %s not found\r\n", source);
  }
}

void cpFile(const char* source, const char* dest) {
  if (SPIFFS.exists(source)) {
    if (SPIFFS.exists(dest)) {
      Serial.printf("  File %s already exists\r\n", dest);
    } else {
      File file = SPIFFS.open(source, FILE_READ);
      File file2 = SPIFFS.open(dest, FILE_WRITE);

      while (file.available()) {
        file2.write(file.read());
      }
    }
  } else {
    Serial.printf("  File %s not found\r\n", fileName);
  }
}

void appendByteFile(fs::FS &fs, const char * path, const byte message){
    Serial.printf("Appending to file: %s\r\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("- failed to open file for appending");
        return;
    }
    if(file.write(message)){
        Serial.println("- message appended");
    } else {
        Serial.println("- append failed");
    }
    file.close();
}

void readLastByteOfFile(fs::FS &fs, const char * path, byte result){
    
    Serial.printf("Opening file: %s\r\n", path);

    File file = fs.open(path, FILE_READ);
    if(!file){
        Serial.println("- failed to open file for appending");
        return;
    }
    while (file.available()) {
      result = file.read();
    }
    file.close();
}

void getLastMessage(char* ligne) {
  // Look for last message in current's day data file
  int idx = 0;

  if (rtc.getYear() != 1970) {
    sprintf(myMessage, "/%s-%04d%02d%02d.txt", idCode, rtc.getYear(), rtc.getMonth() + 1, rtc.getDay());
    File file = SPIFFS.open(myMessage, "r");
    if (file) {
      // Read all file
      while (file.available()) {
        ligne[idx] = file.read();
        if ((ligne[idx] == 10) && (ligne[idx - 1] == 13)) {  // CRLF (Carriage Return & Line Feed)
          ligne[idx - 1] = '\0';
          idx = 0;
        } else {
          idx++;
        }
      }

      if (idx != 0) strcpy(ligne, "1970/01/01 00:00:00 0");
    } else {
      Serial.println("Impossible d ouvrir le fichier du jour");
      // Return 0 value with old date
      strcpy(ligne, "1970/01/01 00:00:00 0");
    }
    file.close();
  } else {
    // In this case date & time are not OK
    strcpy(ligne, "1970/01/01 00:00:00 0");
  }
}

void getLastValueInMessage(char* ligne) {
  // Suppress date and time from ligne
  int idx = 0;
  int nb = 0;  // number of space (CHR(32)) or index for char* values
  char values[500] = "";

  while ((nb != 2) and (ligne[idx] != '\0')) {
    if ((int)ligne[idx] == 32) {
      nb++;
    }
    idx++;
  }
  if (ligne[idx] != '\0') {
    nb = 0;
    while (ligne[idx + nb] != '\0') {
      values[nb] = ligne[idx + nb];
      nb++;
    }
    values[nb] = '\0';
  }
  strcpy(ligne, values);
}

// void putMessageToCurrentDayFile(const char* msg) {
//   // Write msg into specified SPIFFS file
//   sprintf(fileName, "/%s-%04d%02d%02d.txt", mqtt_clientId, rtc.getYear(), rtc.getMonth() + 1, rtc.getDay());
//   File file = SPIFFS.open(fileName, "a");
//   if (!file) {
//     Serial.printf("\n File system is not OK");
//   } else {
//     file.println(msg);
//     file.close();
//   }
// }

// void putMessageToLogFile(const char* msg) {
//   // It is necessary to pollute log with info even if out of time
//   sprintf(fileName, "/%s-%02d.log", mqtt_clientId, rtc.getYear());
//   File file = SPIFFS.open(fileName, "a");
//   if (!file) {
//     Serial.printf("\n   Le systeme de fichier est defaillant ");
//   } else {
//     if (file.size() == 0) {
//       Serial.printf("\n   Le fichier %d a ete cree ", fileName);
//     } else {
//       Serial.printf("\n   Le fichier %d existe => message append ", fileName);
//     }
//     sprintf(myMessage_2, "%04d/%02d/%02d %02d:%02d:%02d %s\n", rtc.getYear(),
//             rtc.getMonth() + 1, rtc.getDay(), rtc.getHour(true), rtc.getMinute(), rtc.getSecond(), msg);
//     file.print(myMessage_2);
//     file.close();
//     Serial.printf("added to log : '%s'", myMessage_2);
//   }
// }

void freeSpiffsSpace(int size) {
}
