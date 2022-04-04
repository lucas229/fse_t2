#include <stdio.h>
#include <signal.h>

#include "mainInterface.h"

void handleSignal(int signal) {
    if(signal == SIGINT) {
        stopServer();
    }
}

int main() {
    signal(SIGINT, &handleSignal);
	initServer();
    return 0;
}
