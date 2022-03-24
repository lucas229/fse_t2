#include <stdio.h>
#include <wiringPi.h>

#include "dht22.h"

int main() {
	wiringPiSetupGpio();
	while(1) {
		printf("Temperatura: %.1f°C\n", get_info('c', 20));
		printf("Umidade: %.1f%%\n\n", get_info('h', 20));
	}
    return 0;
}
