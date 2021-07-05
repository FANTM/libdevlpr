#include <Libdevlpr.h>

// create a DEVLPR object with the pin we have connected on the shield
// can create multiple for different pins if stacking shields
// create this one configured for a 60Hz notch filter (50Hz also available)
Devlpr devlpr(A0, FILTER_60HZ);

void printEMG(Devlpr *d) {
    // grab the most recent EMG values, filtered and raw
    int filtered = d->lastValueCentered(true);
    int raw = d->lastValueCentered(false);
    // we will print four values each time: -1024, 1024, filtered, and raw EMG
    // the -1024 and 1024 simply give us a top and bottom line in the plotter
    Serial.print("-1024 1024 ");
    Serial.println(filtered);
    Serial.print(" ");
    Serial.println(raw);
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
