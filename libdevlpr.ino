#include "Devlpr.h"

Devlpr devlpr;

void printEMG(Devlpr *d) {
    int result = d->lastValueCentered();
    Serial.println(result);
}

void setup() {
    Serial.begin(2000000);
    // add our print function to our DEVLPR schedule
    // try to run once every 1ms
    devlpr.scheduleFunction(printEMG, 1);
}

void loop() {
    // let the DEVLPR library do its job
    devlpr.tick();
}
