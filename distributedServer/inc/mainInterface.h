#ifndef MAIN_INTERFACE
#define MAIN_INTERFACE

void initServer();
void readConfigs(char *text);
void findCountingSensors();
void readSensors();
void checkInputStatus(int *pins, int *size, int index);
char *readFile();
void freeData();

#endif
