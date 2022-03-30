#ifndef JSON_PARSER
#define JSON_PARSER

#include "cJSON.h"

typedef struct NetworkInfo {
    char *centralServerIp;
    int centralServerPort;
    char *distServerIp;
    int distServerPort;
    char *serverName;
} NetworkInfo;

typedef struct Sensor {
    char *type;
    char *tag;
    int gpio;
    int status;
} Sensor;

typedef struct Status {
    int gpio;
    int status;
} Status;

void initJson(char *text);
int parseArray(Sensor **sensors, char *key);
void parseNetworkInfo(NetworkInfo *netInfo);
void parseDhtInfo(Sensor *dht);
void saveSensorData(cJSON *item, Sensor *sensor);
void saveString(cJSON *item, char *key, char **string);
void clearJson();
char *createJson(Sensor *sensors, int *pins, int size, char *key);
int parseStatusArray(Status **statuses, char *key);
char *getType(char *text);
int getPort(char *text);

#endif
