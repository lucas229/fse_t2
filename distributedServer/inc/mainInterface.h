#ifndef MAIN_INTERFACE
#define MAIN_INTERFACE

void initServer();
void listenTcp();
void readConfigs(char *text);
void findCountingSensors();
void *readSensors(void *arg);
void checkInputStatus(int *pins, int *size, int index);
void checkOutputStatus(int *pins, int *size, int index);
char *readFile();
void freeData();
void *readDht(void *arg);
void stopServer();

#endif
