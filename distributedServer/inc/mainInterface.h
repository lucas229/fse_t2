#ifndef MAIN_INTERFACE
#define MAIN_INTERFACE

void initServer();
void *listenTcp(void *arg);
void readConfigs(char *text);
void findCountingSensors();
void readSensors();
void checkInputStatus(int *pins, int *size, int index);
void checkOutputStatus(int *pins, int *size, int index);
char *readFile();
void freeData();

#endif
