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

typedef struct Dht {
    char *type;
    char *tag;
    int gpio;
    float temp;
    float humidity;
} Dht;

typedef struct Status {
    int gpio;
    int status;
} Status;

void initJson(char *text);
int parseArray(Sensor **sensors, char *key);
void parseNetworkInfo(NetworkInfo *netInfo);
void parseDhtInfo(Dht *dht);
void saveSensorData(cJSON *item, Sensor *sensor);
void saveString(cJSON *item, char *key, char **string);
void clearJson();
char *createJson(Sensor *sensors, int *pins, int size, char *key);
char *editJson(char *text, Sensor *sensors, int *pins, int size, char *key);
int parseStatusArray(Status **statuses, char *key);
void addType(char **text, char *type);
void addPort(char **text, int port);
char *createDhtJson(Dht dht);
char *getType(char *text);
char *createType(char *message);

#endif
