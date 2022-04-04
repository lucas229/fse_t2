#include <stdio.h>
#include <unistd.h>
#include <wiringPi.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "mainInterface.h"
#include "jsonParser.h"
#include "cliente_tcp.h"
#include "servidor_tcp.h"
#include "dht22.h"

NetworkInfo netInfo = {0, 0, 0, 0, 0};
Sensor *outputs = NULL;
Sensor *inputs = NULL;
Dht dht = {0, 0, 0, -1, -1};
int outputsSize = 0, inputsSize = 0, entryIndex = -1, exitIndex = -1, stop = 0, running = 0;
pthread_t id1, id2;

void initServer(char *fileName) {
    char *text = readFile(fileName);
    readConfigs(text);

    addType(&text, "Connection");
    addPort(&text, netInfo.distServerPort);
    printf("Aguardando servidor central.\n");
    while(!enviarMensagem(netInfo.centralServerIp, netInfo.centralServerPort, text) && !stop) {
        sleep(1);
    }

    if(!stop) {
        printf("Servidor central conectado.\n");
        free(text);

        findCountingSensors();
        wiringPiSetupGpio();

        pthread_create(&id1, NULL, &readSensors, NULL);
        pthread_create(&id2, NULL, &readDht, NULL);
        running = 1;
        listenTcp();

        pthread_cancel(id1);
        pthread_join(id1, NULL);
        pthread_cancel(id2);
        pthread_join(id2, NULL);
        encerrarServidor();
    }
    freeData();
}

void *readDht(void *arg) {
    while(1) {
        dht.temp = get_info('c', dht.gpio);
        dht.humidity = get_info('h', dht.gpio);
        char *text = createDhtJson(dht);
        addPort(&text, netInfo.distServerPort);
        enviarMensagem(netInfo.centralServerIp, netInfo.centralServerPort, text);
        free(text);
        sleep(1);
    }
}

void listenTcp() {
    configurarServidor(netInfo.distServerPort);
    int stop = 0;
    while(!stop) {
        char *text = aguardarMensagem();
        char *type = getType(text);
        if(strcmp(type, "Sensor") == 0) {
            Status *statuses = NULL;
            initJson(text);
            int size = parseStatusArray(&statuses, "outputs");
            clearJson();
            for(int i = 0; i < size; i++) {
                for(int j = 0; j < outputsSize; j++) {
                    if(statuses[i].gpio == outputs[j].gpio) {
                        pinMode(outputs[j].gpio, OUTPUT);
                        digitalWrite(outputs[j].gpio, statuses[i].status);
                        outputs[j].status = statuses[i].status;
                        break;
                    }
                }
            }
            free(statuses);
        } else if(strcmp(type, "Disconnect") == 0) {
            printf("Servidor central desconectado.\n");
            stop = 1;
        }
        free(text);
        free(type);
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

void *readSensors(void *arg) {
    for(int i = 0; i < inputsSize; i++) {
        pinMode(inputs[i].gpio, INPUT);
    }

    int counter = 1;
    while(1) {
        int inputPins[30] = {0}, outputPins[30] = {0}, inputChanges = 0, outputChanges = 0;
        if(counter == 1) {
            for(int i = 0; i < inputsSize; i++) {
                if(i == entryIndex || i == exitIndex) {
                    continue;
                }
                checkInputStatus(inputPins, &inputChanges, i);
            }

            for(int i = 0; i < outputsSize; i++) {
                checkOutputStatus(outputPins, &outputChanges, i);
            }
        }
        checkInputStatus(inputPins, &inputChanges, entryIndex);
        checkInputStatus(inputPins, &inputChanges, exitIndex);

        if(inputChanges > 0 || outputChanges > 0) {
            char *text = createJson(inputs, inputPins, inputChanges, "inputs");
            text = editJson(text, outputs, outputPins, outputChanges, "outputs");
            addType(&text, "Status");
            addPort(&text, netInfo.distServerPort);
            enviarMensagem(netInfo.centralServerIp, netInfo.centralServerPort, text);
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

void checkOutputStatus(int *pins, int *size, int index) {
    int status = digitalRead(outputs[index].gpio);
    if(outputs[index].status != status) {
        outputs[index].status = !outputs[index].status;
        pins[(*size)++] = index;
    }
}

char *readFile(char *fileName) {

    FILE *file = fopen(fileName, "r");
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

void stopServer() {
    stop = 1;
    if(running) {
        pthread_cancel(id1);
        pthread_join(id1, NULL);
        pthread_cancel(id2);
        pthread_join(id2, NULL);
        encerrarServidor();
    }
    char *text = createType("Disconnect");
    addPort(&text, netInfo.distServerPort);
    enviarMensagem(netInfo.centralServerIp, netInfo.centralServerPort, text);
    free(text);
    freeData();
    exit(0);
}
