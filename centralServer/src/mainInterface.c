#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "mainInterface.h"
#include "jsonParser.h"
#include "servidor_tcp.h"

NetworkInfo netInfo = {0, 0, 0, 0, 0};
Sensor *outputs = NULL;
Sensor *inputs = NULL;
Sensor dht = {0, 0, 0, 0};
int outputsSize = 0, inputsSize = 0, entryIndex = -1, exitIndex = -1;

void initServer() {
    configurarServidor(10052);
    char *text = aguardarMensagem();
    readConfigs(text);
    free(text);

    findCountingSensors();

    while(1) {
        for(int i = 0; i < inputsSize; i++) {
            printf("%s: %d\n", inputs[i].tag, inputs[i].status);
        }
        char *text = aguardarMensagem();
        initJson(text);
        Status *statuses = NULL;
        int size = parseStatusArray(&statuses, "inputs");
        clearJson();
        for(int j = 0; j < size; j++) {
            updateStatus(statuses[j].gpio, statuses[j].status);
        }
        free(statuses);
    }

    freeData();
    encerrarServidor();
}

void updateStatus(int gpio, int status) {
    for(int i = 0; i < inputsSize; i++) {
        if(gpio == inputs[i].gpio) {
            inputs[i].status = status;
            return;
        }
    }
}

void readConfigs(char *text) {
    initJson(text);

    parseNetworkInfo(&netInfo);
    parseDhtInfo(&dht);
    outputsSize = parseArray(&outputs, "outputs");
    inputsSize = parseArray(&inputs, "inputs");

    clearJson();
}

void findCountingSensors() {
    for(int i = 0; i < inputsSize; i++) {
        if(strcmp(inputs[i].type, "contagem") == 0) {
            if(entryIndex == -1) {
                entryIndex = i;
            } else {
                exitIndex = i;
                return;
            }
        }
    }
}

void freeData() {
    free(netInfo.centralServerIp);
    free(netInfo.distServerIp);
    free(netInfo.serverName);
    free(dht.type);
    free(dht.tag);
    for(int i = 0; i < inputsSize; i++) {
        free(inputs[i].type);
        free(inputs[i].tag);
    }
    for(int i = 0; i < outputsSize; i++) {
        free(outputs[i].type);
        free(outputs[i].tag);
    }
    free(inputs);
    free(outputs);
}
