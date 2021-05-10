#include "Devlpr.h"

Devlpr devlpr;

void setup() {
    Serial.begin(9600);
}

void loop() {
    devlpr.tick();
}
