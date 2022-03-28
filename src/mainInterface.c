#include <stdio.h>
#include <unistd.h>
#include <wiringPi.h>
#include <stdlib.h>
#include <string.h>

#include "mainInterface.h"
#include "jsonParser.h"

NetworkInfo netInfo;
Sensor *outputs;
Sensor *inputs;
Sensor dht;
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
    int counter = 11, lastEntry = 0, lastExit = 0;
    while(1) {
        system("clear");
        printf("Contador: %d\n", counter);
        if(counter == 11) {
            for(int i = 0; i < inputsSize; i++) {
                if(i != entryIndex && i != exitIndex) {
                    printf("%s: %d\n", inputs[i].tag, digitalRead(inputs[i].gpio));
                }
            }
            counter = 0;
        }
        int entry = digitalRead(inputs[entryIndex].gpio), exit = digitalRead(inputs[exitIndex].gpio);
        printf("Sensor de entrada de pessoas: %d", entry);
        if(entry != lastEntry) {
            lastEntry = entry;
            printf(" (Mudou)");
        }
        printf("\nSensor de saída de pessoas: %d", exit);
        if(exit != lastExit) {
            lastExit = exit;
            printf(" (Mudou)");
        }
        printf("\n\n");
        counter++;
        usleep(50000);
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
