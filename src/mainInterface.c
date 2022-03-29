#include <stdio.h>
#include <unistd.h>
#include <wiringPi.h>
#include <stdlib.h>
#include <string.h>

#include "mainInterface.h"
#include "jsonParser.h"

NetworkInfo netInfo = {0, 0, 0, 0, 0};
Sensor *outputs = NULL;
Sensor *inputs = NULL;
Sensor dht = {0, 0, 0, 0};
int outputsSize = 0, inputsSize = 0, entryIndex = -1, exitIndex = -1;

void initServer() {
    readConfigs();
    findCountingSensors();
    wiringPiSetupGpio();
    readSensors();
    freeData();
}

void readConfigs() {
    char *text = readFile();
    initJson(text);
    free(text);

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

void readSensors() {
    for(int i = 0; i < inputsSize; i++) {
        pinMode(inputs[i].gpio, INPUT);
    }

    int counter = 1;
    while(1) {
        int pins[30], size = 0;
        if(counter == 1) {
            for(int i = 0; i < inputsSize; i++) {
                if(i == entryIndex || i == exitIndex) {
                    continue;
                }
                checkInputStatus(pins, &size, i);
            }
        }
        checkInputStatus(pins, &size, entryIndex);
        checkInputStatus(pins, &size, exitIndex);

        if(size > 0) {
            char *text = createJson(inputs, pins, size, "inputs");
            printf("\n__________________________________________\n%s\n__________________________________________\n", text);
            free(text);
        }

        counter++;
        if(counter == 11) {
            counter = 1;
        }
        usleep(50000);
    }
}

void checkInputStatus(int *pins, int *size, int index) {
    int status = digitalRead(inputs[index].gpio);
    if(inputs[index].status != status) {
        inputs[index].status = !inputs[index].status;
        if(status == 0 && (index == entryIndex || index == exitIndex)) {
            return;
        }
        pins[(*size)++] = index;
    }
}

char *readFile() {
    FILE *file = fopen("configs/configuracao_andar_terreo.json", "r");
    if(file == NULL) {
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    char *text = calloc(size + 1, sizeof(char));
    fread(text, sizeof(char), size, file);
    fclose(file);

    return text;
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
