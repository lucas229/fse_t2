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
#include "logger.h"

NetworkInfo netInfo[MAX];
Sensor *outputs[MAX];
Sensor *inputs[MAX];
Dht dht[MAX];
int outputsSize[MAX] = {0}, inputsSize[MAX] = {0}, entryIndex[MAX] = {-1}, exitIndex[MAX] = {-1}, connections = 0;
int stop = 0, selectedServer = -1;
pthread_t id1 = -1, id2 = -1, id3 = -1, alarmThread = -1, userAlarmThread;
int totalCounting = 0, secondCounting = 0, firstPort = -1, secondPort = -1;
int sprinklerServer = -1;
int alarmSound[MAX] = {0}, userAlarm = 0;

void initServer() {
    for(int i = 0; i < MAX; i++) {
        entryIndex[i] = -1;
        exitIndex[i] = -1;
    }
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
        } else if(strcmp(type, "DHT") == 0) {
            int port = getPort(text);
            readDhtInfo(&dht[findByPort(port)], text);
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

    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);

    while(1) {
        erase();

        attron(A_BOLD);
        printw("Selecione um servidor distribuído:\n");
        attroff(A_BOLD);

        for(int i = 0; i < connections; i++) {
            printw("[%d] %s\n", i + 1, netInfo[i].serverName);
        }
        
        if(connections == 0) {
            printw("\nNão há servidores disponíveis no momento.\n");
        }

        if(userAlarm) {
            attron(COLOR_PAIR(1));
            printw("\n[%d] Alarme: ON\n", connections + 1);
        } else {
            attron(COLOR_PAIR(2));
            printw("\n[%d] Alarme: OFF\n", connections + 1);
        }

        attroff(COLOR_PAIR(1));
        attroff(COLOR_PAIR(2));

        if(firstPort != -1) {
            printw("\nPessoas no predio: %d\n", totalCounting);
            printw("Pessoas 1o andar: %d\n", totalCounting - secondCounting);
        }
        if(secondPort != -1) {
            printw("Pessoas 2o andar: %d\n", secondCounting);
        }

        if(isAlarmOn()) {
            attron(COLOR_PAIR(1));
            printw("\nAlarme de incendio: ON\n");
        } else {
            attron(COLOR_PAIR(2));
            printw("\nAlarme de incendio: OFF\n");
        }
        attroff(COLOR_PAIR(1));
        attroff(COLOR_PAIR(2));

        refresh();

        int command = getch();
        if(command != ERR) {
            if(command == 'q') {
                break;
            }
            command -= '0';
            command--;
            if(command == connections) {
                if(userAlarm == 1 || userAlarm == 2) {
                    userAlarm = 0;
                    pthread_join(userAlarmThread, NULL);
                    logData(NULL, "Alarme de usuario", 0);
                } else if(userAlarm == 0) {
                    if(!isAlarmDeviceActive()) {
                        userAlarm = 1;
                        logData(NULL, "Alarme de usuario", 3);
                    }
                }
            } else if(command >= 0 && command < connections) {
                selectedServer = command;
                serverMenu();
                timeout(1000);
            }
        }
    }
    timeout(-1);
}

int isAlarmDeviceActive() {
    for(int i = 0; i < connections; i++) {
        for(int j = 0; j < inputsSize[i]; j++) {
            if(strcmp(inputs[i][j].type, "janela") == 0 || strcmp(inputs[i][j].type, "porta") == 0 || strcmp(inputs[i][j].type, "presenca") == 0) {
                if(inputs[i][j].status == 1) {
                    return 1;
                }
            }
        }
    }
    return 0;
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

        if(isAlarmOn()) {
            attron(COLOR_PAIR(1));
            mvprintw(row++, 0, "Alarme de incendio: ON");
        } else {
            attron(COLOR_PAIR(2));
            mvprintw(row++, 0, "Alarme de incendio: OFF");
        }

        attroff(COLOR_PAIR(1));
        attroff(COLOR_PAIR(2));

        if(dht[selectedServer].temp != -1) {
            mvprintw(row++, 0, "Temperatura: %.1fºC", dht[selectedServer].temp);
        }
        if(dht[selectedServer].humidity != -1) {
            mvprintw(row++, 0, "Umidade: %.1f%%", dht[selectedServer].humidity);
        }

        if(firstPort != -1) {
            mvprintw(row++, 0, "Pessoas no predio: %d", totalCounting);
            mvprintw(row++, 0, "Pessoas 1o andar: %d", totalCounting - secondCounting);
        }
        if(secondPort != -1) {
            mvprintw(row++, 0, "Pessoas 2o andar: %d", secondCounting);
        }

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
        logData(netInfo[selectedServer].serverName, outputs[selectedServer][pins[i]].tag, outputs[selectedServer][pins[i]].status);
    }
    enviarMensagem(netInfo[selectedServer].distServerIp, netInfo[selectedServer].distServerPort, text);
    free(text);
}

void changeStatus(int pin) {
    int pins[] = {pin};
    char *text = createOutputsJson(&outputs[selectedServer][0], pins, 1, "outputs", -1);
    outputs[selectedServer][pin].status = !outputs[selectedServer][pin].status;
    logData(netInfo[selectedServer].serverName, outputs[selectedServer][pin].tag, outputs[selectedServer][pin].status);
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
    findPorts();
    findSprinklerSensor();
    connections++;
}

void findPorts() {
    if(strcmp(netInfo[connections].serverName, "Térreo") == 0) {
        firstPort = netInfo[connections].distServerPort;
    } else if(strcmp(netInfo[connections].serverName, "1º Andar") == 0) {
        secondPort = netInfo[connections].distServerPort;
    }
}

void updateStatuses(int port, char *key) {
    Status *statuses = NULL;
    int size = parseStatusArray(&statuses, key);
    int portIndex = findByPort(port);
    for(int j = 0; j < size; j++) {
        if(strcmp(key, "inputs") == 0) {
            updateStatus(statuses[j].gpio, statuses[j].status, portIndex);

            if(port == firstPort) {
                int gpio = findByGpio(statuses[j].gpio, findByPort(firstPort));
                if(gpio == entryIndex[findByPort(firstPort)]) {
                    totalCounting++;
                } else if(gpio == exitIndex[findByPort(firstPort)]) {
                    totalCounting--;
                }
            } else if(port == secondPort) {
                int gpio = findByGpio(statuses[j].gpio, findByPort(secondPort));
                if(gpio == entryIndex[findByPort(secondPort)]) {
                    secondCounting++;
                } else if(gpio == exitIndex[findByPort(secondPort)]) {
                    secondCounting--;
                }
            }

            if(strcmp(inputs[portIndex][findByGpio(statuses[j].gpio, portIndex)].type, "fumaca") == 0) {
                if(statuses[j].status == 1) {
                    if(sprinklerServer != -1) {
                        int pin = findPin("aspersor", sprinklerServer);
                        int pins[] = {pin};
                        char *text = createOutputsJson(&outputs[sprinklerServer][0], pins, 1, "outputs", 1);
                        outputs[sprinklerServer][pin].status = !outputs[sprinklerServer][pin].status;
                        enviarMensagem(netInfo[sprinklerServer].distServerIp, netInfo[sprinklerServer].distServerPort, text);
                        free(text);
                        if(!isAlarmOn()) {
                            pthread_create(&alarmThread, NULL, &playAlarm, NULL);
                        }
                        alarmSound[portIndex] = 1;
                        logData(NULL, "Alarme de incendio", 1);
                    }
                } else {
                    alarmSound[portIndex] = 0;
                    logData(NULL, "Alarme de incendio", 0);
                    if(!isAlarmOn()) {
                        pthread_join(alarmThread , NULL);
                    }
                }
            }

            if(userAlarm == 1) {
                char type[100];
                strcpy(type, inputs[portIndex][findByGpio(statuses[j].gpio, portIndex)].type);
                if(strcmp(type, "janela") == 0 || strcmp(type, "porta") == 0 || strcmp(type, "presenca") == 0) {
                    userAlarm = 2;
                    pthread_create(&userAlarmThread, NULL, &playUserAlarm, NULL);
                    logData(NULL, "Alarme de usuario", 1);
                }
            }

        } else if(strcmp(key, "outputs") == 0) {
            updateOutputStatus(statuses[j].gpio, statuses[j].status, portIndex);
        }
    }
    free(statuses);
}

int isAlarmOn() {
    for(int i = 0; i < connections; i++) {
        if(alarmSound[i]) {
            return 1;
        }
    }
    return 0;
}

void *playAlarm(void *arg) {
    while(isAlarmOn()) {
        beep();
        sleep(1);
    }
    return NULL;
}

void *playUserAlarm(void *arg) {
    while(userAlarm == 2) {
        beep();
        sleep(1);
    }
    return NULL;
}

int findPin(char *key, int server) {
    for(int i = 0; i < outputsSize[server]; i++) {
        if(strcmp(outputs[server][i].type, key) == 0) {
            return i;
        }
    }
    return -1;
}

int findByGpio(int gpio, int server) {
    for(int i = 0; i < inputsSize[server]; i++) {
        if(inputs[server][i].gpio == gpio) {
            return i;
        }
    }
    return -1;
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

void findSprinklerSensor() {
    if(sprinklerServer != -1) {
        return;
    }
    for(int i = 0; i < outputsSize[connections]; i++) {
        if(strcmp(outputs[connections][i].type, "aspersor") == 0) {
            sprinklerServer = connections;
            break;
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
