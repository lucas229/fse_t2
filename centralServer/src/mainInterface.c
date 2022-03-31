#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <ncurses.h>

#include "mainInterface.h"
#include "jsonParser.h"
#include "servidor_tcp.h"
#include "cliente_tcp.h"

NetworkInfo netInfo[MAX];
Sensor *outputs[MAX];
Sensor *inputs[MAX];
Sensor dht[MAX];
int outputsSize[MAX] = {0}, inputsSize[MAX] = {0}, entryIndex[MAX] = {-1}, exitIndex[MAX] = {-1}, connections = 0;

void initServer() {
    initscr();

    pthread_t id1, id2, id3;
    pthread_create(&id1, NULL, &listenTcp, NULL);
    pthread_create(&id2, NULL, &initMenu, NULL);
    pthread_create(&id3, NULL, &waitCommand, NULL);
    pthread_join(id1, NULL);
    pthread_join(id2, NULL);
    pthread_join(id3, NULL);

    freeData();
    encerrarServidor();
}

void *listenTcp(void *arg) {
    configurarServidor(10052);
    while(1) {
        char *text = aguardarMensagem();
        char *type = getType(text);
        if(strcmp(type, "Connection") == 0) {
            addConnection(text);
        } else if(strcmp(type, "Status") == 0) {
            int port = getPort(text);
            initJson(text);
            updateStatuses(port, "inputs");
            updateStatuses(port, "outputs");
            clearJson();
        }
        free(text);
        free(type);
    }
}

void *initMenu(void *arg) {
    noecho();
    while(1) {
        clear();

        attron(A_BOLD);
        if(connections) {
            mvprintw(0, 0, "Servidor Distribuído: %s", netInfo[0].serverName);
            mvprintw(2, 0, "Estados");
            mvprintw(2, 60, "Acionamento");
            attroff(A_BOLD);
        }

        for(int i = 0; i < inputsSize[0]; i++) {
            if(strcmp("contagem", inputs[0][i].type) != 0) {
                mvprintw(i + 3, 0, "%s: %d", inputs[0][i].tag, inputs[0][i].status);
            }
        }
        for(int i = 0; i < outputsSize[0]; i++) {
            mvprintw(i + 3 + inputsSize[0], 0, "%s: %d", outputs[0][i].tag, outputs[0][i].status);
        }

        for(int i = 0; i < outputsSize[0]; i++) {
            mvprintw(i + 3, 60, "[%d] Ligar/Desligar %s", i + 1, outputs[0][i].tag, outputs[0][i].status);
        }

        refresh();
        sleep(1);
    }
    endwin();
}

void *waitCommand(void *arg) {
    while(1) {
        int command = getch();
        command -= '0';
        int pins[] = {command - 1};
        char *text = createOutputsJson(&outputs[0][0], pins, 1, "outputs");
        outputs[0][command - 1].status = !outputs[0][command - 1].status;
        enviarMensagem(netInfo[0].distServerIp, netInfo[0].distServerPort, text);
        free(text);
    }
}

void addConnection(char *text) {
    readConfigs(text);
    findCountingSensors();
    connections++;
}

void updateStatuses(int port, char *key) {
    Status *statuses = NULL;
    int size = parseStatusArray(&statuses, key);
    for(int j = 0; j < size; j++) {
        if(strcmp(key, "inputs") == 0) {
            updateStatus(statuses[j].gpio, statuses[j].status, findByPort(port));
        } else if(strcmp(key, "outputs") == 0) {
            updateOutputStatus(statuses[j].gpio, statuses[j].status, findByPort(port));
        }
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

void updateOutputStatus(int gpio, int status, int index) {
    for(int i = 0; i < outputsSize[index]; i++) {
        if(gpio == outputs[index][i].gpio) {
            outputs[index][i].status = status;
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
