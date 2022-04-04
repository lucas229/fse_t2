#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "jsonParser.h"

cJSON *root = NULL;

void initJson(char *text) {
    root = cJSON_Parse(text);
}

int parseArray(Sensor **sensors, char *key) {
    cJSON *array = cJSON_GetObjectItem(root, key);
    int size = cJSON_GetArraySize(array);
    *sensors = calloc(size, sizeof(Sensor));
    for(int i = 0; i < size; i++) {
        cJSON *item = cJSON_GetArrayItem(array, i);
        saveSensorData(item, *sensors + i);
    }
    return size;
}

void parseNetworkInfo(NetworkInfo *netInfo) {
    saveString(root, "ip_servidor_central", &netInfo->centralServerIp);
    saveString(root, "ip_servidor_distribuido", &netInfo->distServerIp);
    saveString(root, "nome", &netInfo->serverName);
    netInfo->centralServerPort = cJSON_GetObjectItem(root, "porta_servidor_central")->valueint;
    netInfo->distServerPort = cJSON_GetObjectItem(root, "porta_servidor_distribuido")->valueint;
}

void parseDhtInfo(Dht *dht) {
    cJSON *array = cJSON_GetObjectItem(root, "sensor_temperatura");
    cJSON *item = cJSON_GetArrayItem(array, 0);
    saveString(item, "type", &dht->type);
    saveString(item, "tag", &dht->tag);
    dht->gpio = cJSON_GetObjectItem(item, "gpio")->valueint;
    dht->temp = -1;
    dht->humidity = -1;
}

void saveSensorData(cJSON *item, Sensor *sensor) {
    saveString(item, "type", &sensor->type);
    saveString(item, "tag", &sensor->tag);
    sensor->gpio = cJSON_GetObjectItem(item, "gpio")->valueint;
}

void saveString(cJSON *item, char *key, char **string) {
    cJSON *subItem = cJSON_GetObjectItem(item, key);
    *string = subItem->valuestring;
    subItem->valuestring = NULL;
}

void clearJson() {
    cJSON_Delete(root);
    root = NULL;
}

char *createJson(Sensor *sensors, int *pins, int size, char *key) {
    cJSON *root = cJSON_CreateObject();
    cJSON *items = cJSON_CreateArray();

    cJSON_AddItemToObject(root, key, items);

    for(int i = 0; i < size; i++) {
        cJSON *item = cJSON_CreateObject();
        cJSON_AddItemToArray(items, item);
        cJSON_AddItemToObject(item, "gpio", cJSON_CreateNumber(sensors[pins[i]].gpio));
        cJSON_AddItemToObject(item, "status", cJSON_CreateNumber(sensors[pins[i]].status));
    }

    char *out = cJSON_Print(root);

    cJSON_Delete(root);

    return out;
}

char *editJson(char *text, Sensor *sensors, int *pins, int size, char *key) {
    cJSON *root = cJSON_Parse(text);
    cJSON *items = cJSON_CreateArray();

    cJSON_AddItemToObject(root, key, items);

    for(int i = 0; i < size; i++) {
        cJSON *item = cJSON_CreateObject();
        cJSON_AddItemToArray(items, item);
        cJSON_AddItemToObject(item, "gpio", cJSON_CreateNumber(sensors[pins[i]].gpio));
        cJSON_AddItemToObject(item, "status", cJSON_CreateNumber(sensors[pins[i]].status));
    }

    free(text);
    char *out = cJSON_Print(root);

    cJSON_Delete(root);

    return out;
}

int parseStatusArray(Status **statuses, char *key) {
    cJSON *array = cJSON_GetObjectItem(root, key);
    int size = cJSON_GetArraySize(array);
    *statuses = calloc(size, sizeof(Status));
    for(int i = 0; i < size; i++) {
        cJSON *item = cJSON_GetArrayItem(array, i);
        (*statuses)[i].gpio = cJSON_GetObjectItem(item, "gpio")->valueint;
        (*statuses)[i].status = cJSON_GetObjectItem(item, "status")->valueint;
    }
    return size;
}

void addType(char **text, char *type) {
    cJSON *root = cJSON_Parse(*text);
    cJSON_AddItemToObject(root, "type", cJSON_CreateString(type));
    free(*text);
    *text = cJSON_Print(root);
    cJSON_Delete(root);
}

void addPort(char **text, int port) {
    cJSON *root = cJSON_Parse(*text);
    cJSON_AddItemToObject(root, "port", cJSON_CreateNumber(port));
    free(*text);
    *text = cJSON_Print(root);
    cJSON_Delete(root);
}

char *createDhtJson(Dht dht) {
    cJSON *root = cJSON_CreateObject();

    cJSON_AddItemToObject(root, "Type", cJSON_CreateString("DHT"));
    cJSON_AddItemToObject(root, "temperature", cJSON_CreateNumber(dht.temp));
    cJSON_AddItemToObject(root, "humidity", cJSON_CreateNumber(dht.humidity));

    char *text = cJSON_Print(root);

    cJSON_Delete(root);

    return text;  
}

char *getType(char *text) {
    cJSON *root = cJSON_Parse(text);
    char *type = NULL;
    saveString(root, "type", &type);
    cJSON_Delete(root);
    return type;
}

char *createType(char *message) {
    cJSON *root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "type", cJSON_CreateString(message));
    char *out = cJSON_Print(root);
    cJSON_Delete(root);
    return out;
}
