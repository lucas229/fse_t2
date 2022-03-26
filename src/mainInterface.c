#include <stdio.h>
#include <unistd.h>
#include <wiringPi.h>

#include "mainInterface.h"

void initServer() {
    wiringPiSetupGpio();
    readSensors();
}

void readSensors() {
    int presenceSensor = 26, smokeSensor = 23, windowSensor1 = 9, windowSensor2 = 11, doorSensor = 10, entryCountingSensor = 13, exitCountingSensor = 19;
    pinMode(presenceSensor, INPUT);
    pinMode(smokeSensor, INPUT);
    pinMode(windowSensor1, INPUT);
    pinMode(windowSensor2, INPUT);
    pinMode(doorSensor, INPUT);
    pinMode(entryCountingSensor, INPUT);
    pinMode(exitCountingSensor, INPUT);
    int counter = 11, lastEntry = 0, lastExit = 0;
    while(1) {
        printf("Contador: %d\n", counter);
        if(counter == 11) {
            printf("Sensor de presença: %d\n", digitalRead(presenceSensor));
            printf("Sensor de fumaça: %d\n", digitalRead(smokeSensor));
            printf("Sensor de janela 1: %d\n", digitalRead(windowSensor1));
            printf("Sensor de janela 2: %d\n", digitalRead(windowSensor2));
            printf("Sensor de porta de entrada: %d\n", digitalRead(doorSensor));
            counter = 0;
        }
        int entry = digitalRead(entryCountingSensor), exit = digitalRead(exitCountingSensor);
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
