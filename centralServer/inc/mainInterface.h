#ifndef MAIN_INTERFACE
#define MAIN_INTERFACE

void initServer();
void readConfigs();
void findCountingSensors();
void checkInputStatus(int *pins, int *size, int index);
char *readFile();
void freeData();
void updateStatus(int gpio, int status);

#endif
