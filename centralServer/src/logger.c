#include <stdio.h>
#include <time.h>

void logData(char *server, char *item, int status) {
    FILE *file = fopen("Logs/logs.csv", "a");
    time_t now;
    time(&now);
    struct tm *local = localtime(&now);

    char time[100];
    strftime(time, sizeof(time), "%d/%m/%Y, %H:%M:%S, ", local);
    fprintf(file, time);

    if(server != NULL) {
        fprintf(file, "%s: ", server);
    }
    if(status == 1) {
        fprintf(file, "Ligar");
    } else if(status == 0) {
        fprintf(file, "Desligar");
    } else if(status == 3) {
        fprintf(file, "Ativar");
    }
    fprintf(file, " %s.\n", item);
    fclose(file);
}
