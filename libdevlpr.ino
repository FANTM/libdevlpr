#include "Devlpr.h"

Devlpr devlpr;

void setup() {
    Serial.begin(2000000);
}

unsigned long microsSincePrint = 0;
unsigned long prevLoopMicros = 0;
int result = 0;
void loop() {
    // let the DEVLPR library do its job
    devlpr.tick();

    // try to sample and print data at 1000Hz
    unsigned long currMicros = micros();
    unsigned long microsDelta = currMicros - prevLoopMicros;
    microsSincePrint += microsDelta;
    // has 1ms passed
    if (microsSincePrint > 1000) {
        // 1ms has passed, get the value
        result = devlpr.lastValueCentered();
        Serial.println(result);
        // update our last filtered call
        microsSincePrint = 0;
    }
    // update our last loop time
    prevLoopMicros = currMicros;
}
