#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "mainInterface.h"
#include "jsonParser.h"
#include "servidor_tcp.h"

NetworkInfo netInfo[MAX];
Sensor *outputs[MAX];
Sensor *inputs[MAX];
Sensor dht[MAX];
int outputsSize[MAX] = {0}, inputsSize[MAX] = {0}, entryIndex[MAX] = {-1}, exitIndex[MAX] = {-1}, connections = 0;

void initServer() {
    configurarServidor(10052);

    while(1) {
        for(int i = 0; i < connections; i++) {
            printf("Servidor: %s\n", netInfo[i].serverName);
            for(int j = 0; j < inputsSize[i]; j++) {
                printf("%s: %d\n", inputs[i][j].tag, inputs[i][j].status);
            }
            printf("\n");
        }
        char *text = aguardarMensagem();
        char *type = getType(text);
        if(strcmp(type, "Connection") == 0) {
            addConnection(text);
        } else if(strcmp(type, "Status") == 0) {
            int port = getPort(text);
            initJson(text);
            updateStatuses(port);
            clearJson();
        }
        free(text);
        free(type);
    }

    freeData();
    encerrarServidor();
}

void addConnection(char *text) {
    readConfigs(text);
    findCountingSensors();
    connections++;
}

void updateStatuses(int port) {
    Status *statuses = NULL;
    int size = parseStatusArray(&statuses, "inputs");
    for(int j = 0; j < size; j++) {
        updateStatus(statuses[j].gpio, statuses[j].status, findByPort(port));
    }
    free(statuses);
}

void updateStatus(int gpio, int status, int index) {
    for(int i = 0; i < inputsSize[index]; i++) {
        if(gpio == inputs[index][i].gpio) {
            inputs[index][i].status = status;
            return;
        }
    }
}

int findByPort(int port) {
    for(int i = 0; i < connections; i++) {
        if(netInfo[i].distServerPort == port) {
            return i;
        }
    }
    return -1;
}

void readConfigs(char *text) {
    initJson(text);

    parseNetworkInfo(&netInfo[connections]);
    parseDhtInfo(&dht[connections]);
    outputsSize[connections] = parseArray(&outputs[connections], "outputs");
    inputsSize[connections] = parseArray(&inputs[connections], "inputs");

    clearJson();
}

void findCountingSensors() {
    for(int i = 0; i < inputsSize[connections]; i++) {
        if(strcmp(inputs[connections][i].type, "contagem") == 0) {
            if(entryIndex[connections] == -1) {
                entryIndex[connections] = i;
            } else {
                exitIndex[connections] = i;
                return;
            }
        }
    }
}

void freeData() {
    for(int i = 0; i < connections; i++) {
        free(netInfo[i].centralServerIp);
        free(netInfo[i].distServerIp);
        free(netInfo[i].serverName);
        free(dht[i].type);
        free(dht[i].tag);
    }

    for(int i = 0; i < connections; i++) {
        for(int j = 0; j < inputsSize[i]; j++) {
            free(inputs[i][j].type);
            free(inputs[i][j].tag);
        }
        for(int j = 0; j < outputsSize[i]; j++) {
            free(outputs[i][j].type);
            free(outputs[i][j].tag);
        }
        free(inputs[i]);
        free(outputs[i]);
    }
}
