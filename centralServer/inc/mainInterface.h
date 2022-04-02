#ifndef MAIN_INTERFACE
#define MAIN_INTERFACE

#define MAX 3

void initServer();
void *listenTcp(void *arg);
void *initMenu(void *arg);
void serverSelectionMenu();
void serverMenu();
void changeStatus(int pin);
void exitServer();
void addConnection(char *text);
void updateStatuses(int port, char *key);
void updateStatus(int gpio, int status, int index);
void updateOutputStatus(int gpio, int status, int index);
int findByPort(int port);
void readConfigs();
void findCountingSensors();
void freeData();
void enableDevices(char *key, int status);
int findByGpio(int gpio, int server);
void findPorts();

#endif
