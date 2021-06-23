#include <libdevlpr.h>

// create a DEVLPR object with the pin we have connected on the shield
// can create multiple for different pins if stacking shields
Devlpr devlpr(A0);

void printEMG(Devlpr *d) {
    // first grab the most recent raw EMG value sampled and then centered
    // this method subtracts a window average to roughly center around 0
    int result = d->lastValueCentered();
    // we will print three values each time: -1024, 1024, and the measurement
    // the -1024 and 1024 simply give us a top and bottom line in the plotter
    Serial.print("-1024 1024 ");
    Serial.println(result);
}

void setup() {
    // prepare Serial for printing to the plotter
    Serial.begin(2000000);
    // add a printing function as a scheduled callback for our Devlpr
    // here we set the function to be called every 1ms
    devlpr.scheduleFunction(printEMG, 1);
}

void loop() {
    // let the DEVLPR library do its job
    devlpr.tick();
}
