#ifndef PROJECT_SPIFFS_H
#define PROJECT_SPIFFS_H

#include <cstdint>

void rmFile(const char* fileName);
void mvFile(const char* source, const char* dest);
void cpFile(const char* source, const char* dest);

void printSpiffsInfo();
void printDf();
void getSpiffsLs(char* listDir, const char* dirname);
void getLsSorted(char* listDir);
void printSpiffsList(const char* dirname, uint8_t levels);

bool enoughFreeSpaceOnSpiffs(const int limite);
void readFile(const char* path);
void printSpiffsFileContent(const char* fileName);
void appendByteFile(fs::FS &fs, const char * path, const byte message);
void readLastByteOfFile(fs::FS &fs, const char * path, byte result);

// void getLastMessage(char* ligne);
// void getLastValueInMessage(char* ligne);
void putMessageToCurrentDayFile(const char* msg);
void putMessageToLogFile(const char* msg);
void freeSpiffsSpace(int size);
#endif