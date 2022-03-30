#ifndef DHT22
#define DHT22

#include <stdint.h>

// CONSTANTS
#define MAX_TIMINGS	85
#define WAIT_TIME 2000

// FUNCTION DECLARATIONS
int read_dht_data();
float get_info(char selectedMode, uint8_t pin);

#endif
