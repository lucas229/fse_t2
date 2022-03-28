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
    *sensors = malloc(size * sizeof(Sensor));
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

void parseDhtInfo(Sensor *dht) {
    cJSON *array = cJSON_GetObjectItem(root, "sensor_temepratura");
    cJSON *item = cJSON_GetArrayItem(array, 0);
    saveString(item, "model", &dht->type);
    saveString(item, "tag", &dht->tag);
    dht->gpio = cJSON_GetObjectItem(item, "gpio")->valueint;
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
