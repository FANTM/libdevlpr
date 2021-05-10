#include "Devlpr.h"

Devlpr devlpr;

void setup() {
    Serial.begin(9600);
}

unsigned long microsSinceFilteredCall = 0;
unsigned long prevLoopMicros = 0;
float result = 0.0;
void loop() {
    // let the DEVLPR library do its job
    devlpr.tick();

    // try to sample filtered data at 1000Hz
    unsigned long currMicros = micros();
    unsigned long microsDelta = currMicros - prevLoopMicros;
    microsSinceFilteredCall += microsDelta;
    // has 1ms passed
    if (microsSinceFilteredCall > 1000) {
        // 1ms has passed, get the value
        result = devlpr.lastValueFiltered();
        Serial.println(result);
        // update our last filtered call
        microsSinceFilteredCall = 0;
    }
    // update our last loop time
    prevLoopMicros = currMicros;
}
