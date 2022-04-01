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
int stop = 0, selectedServer = -1;
pthread_t id1 = -1, id2 = -1, id3 = -1;

void initServer() {
    pthread_create(&id1, NULL, &listenTcp, NULL);
    pthread_create(&id2, NULL, &initMenu, NULL);
    pthread_join(id2, NULL);
    exitServer();
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
    initscr();
    noecho();
    curs_set(0);
    serverSelectionMenu();
    endwin();
    return NULL;
}

void serverSelectionMenu() {
    timeout(1000);
    while(1) {
        erase();

        attron(A_BOLD);
        printw("Selecione um servidor distribuído:\n\n");
        attroff(A_BOLD);

        for(int i = 0; i < connections; i++) {
            printw("[%d] %s\n", i + 1, netInfo[i].serverName);
        }
        if(connections == 0) {
            printw("Não há servidores disponíveis no momento.");
        }

        refresh();

        int command = getch();
        if(command != ERR) {
            if(command == 'q') {
                break;
            }
            command -= '0';
            selectedServer = command - 1;
            serverMenu();
        }
    }
    timeout(-1);
}

void serverMenu() {
    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);

    cbreak();
    timeout(500);
    while(1) {
        erase();

        int row = 0;
        attron(A_BOLD);
        mvprintw(row, 0, "Servidor Distribuído: %s", netInfo[selectedServer].serverName);
        row += 2;
        mvprintw(row, 0, "Estados");
        mvprintw(row++, 40, "Acionamento");
        attroff(A_BOLD);

        for(int i = 0; i < inputsSize[selectedServer]; i++) {
            if(strcmp("contagem", inputs[selectedServer][i].type) != 0) {
                if(inputs[selectedServer][i].status) {
                    attron(COLOR_PAIR(1));
                } else {
                    attron(COLOR_PAIR(2));
                }
                mvprintw(row++, 0, "%s: %d", inputs[selectedServer][i].tag, inputs[selectedServer][i].status);
            }
        }
        for(int i = 0; i < outputsSize[selectedServer]; i++) {
            if(outputs[selectedServer][i].status) {
                attron(COLOR_PAIR(1));
            } else {
                attron(COLOR_PAIR(2));
            }
            mvprintw(row++, 0, "%s: %d", outputs[selectedServer][i].tag, outputs[selectedServer][i].status);
        }

        attroff(COLOR_PAIR(1));
        attroff(COLOR_PAIR(2));

        row = 3;
        for(int i = 0; i < outputsSize[selectedServer]; i++) {
            mvprintw(row++, 40, "[%d] Ligar/Desligar %s", i + 1, outputs[selectedServer][i].tag, outputs[selectedServer][i].status);
        }
        mvprintw(row++, 40, "[%d] Ligar todas as lâmpadas", outputsSize[selectedServer] + 1);
        mvprintw(row++, 40, "[%d] Desligar todas as lâmpadas", outputsSize[selectedServer] + 2);
        mvprintw(row++, 40, "[%d] Ligar todos os ares-condicionados", outputsSize[selectedServer] + 3);
        mvprintw(row++, 40, "[%d] Desligar todos os ares-condicionados", outputsSize[selectedServer] + 4);

        refresh();
        
        int command;
        if((command = getch()) != ERR) {
            if(command == 'q') {
                break;
            } else {
                command -= '0';
                if(command >= 1 && command <= outputsSize[selectedServer]) {
                    changeStatus(command - 1);
                } else if(command == outputsSize[selectedServer] + 1) {
                    enableDevices("lampada", 1);
                } else if(command == outputsSize[selectedServer] + 2) {
                    enableDevices("lampada", 0);
                } else if(command == outputsSize[selectedServer] + 3) {
                    enableDevices("ar-condicionado", 1);
                } else if(command == outputsSize[selectedServer] + 4) {
                    enableDevices("ar-condicionado", 0);
                }
            }
        }
    }
    timeout(-1);
}

void enableDevices(char *key, int status) {
    int n = 0, pins[30] = {0};
    for(int i = 0; i < outputsSize[selectedServer]; i++) {
        if(strcmp(outputs[selectedServer][i].type, key) == 0) {
            pins[n++] = i;
        }
    }
    char *text = createOutputsJson(&outputs[selectedServer][0], pins, n, "outputs", status);
    for(int i = 0; i < n; i++) {
        outputs[selectedServer][pins[i]].status = status;
    }
    enviarMensagem(netInfo[selectedServer].distServerIp, netInfo[selectedServer].distServerPort, text);
    free(text);
}

void changeStatus(int pin) {
    int pins[] = {pin};
    char *text = createOutputsJson(&outputs[selectedServer][0], pins, 1, "outputs", -1);
    outputs[selectedServer][pin].status = !outputs[selectedServer][pin].status;
    enviarMensagem(netInfo[selectedServer].distServerIp, netInfo[selectedServer].distServerPort, text);
    free(text);
}

void exitServer() {
    pthread_cancel(id1);
    pthread_join(id1, NULL);
    endwin();
    encerrarServidor();
    freeData();
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
