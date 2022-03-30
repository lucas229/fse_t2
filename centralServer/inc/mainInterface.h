#ifndef MAIN_INTERFACE
#define MAIN_INTERFACE

#define MAX 5

void initServer();
void readConfigs();
void findCountingSensors();
void checkInputStatus(int *pins, int *size, int index);
char *readFile();
void freeData();
void updateStatus(int gpio, int status, int index);
void addConnection(char *text);
void updateStatuses(int port);
int findByPort(int port);

#endif
