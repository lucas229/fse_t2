#include <stdio.h>
#include <signal.h>

#include "mainInterface.h"

void handleSignal(int signal) {
    printf("\nEncerrando.\n");
    if(signal == SIGINT) {
        stopServer();
    }
}

int main(int argc, char *argv[]) {
    if(argc != 2) {
        printf("Forma correta: %s make run JSON=<nome_do_arquivo.json>\n", argv[0]);
    } else {
        signal(SIGINT, &handleSignal);
	    initServer(argv[1]);
    }
    return 0;
}
