#include <libdevlpr.h>

// create a DEVLPR object with the pin we have connected on the shield
// can create multiple for different pins if stacking shields
Devlpr devlpr(A0);

void printAvg(Devlpr *d) {
    // first grab the most recent window average of EMG values sampled
    // this is effectively a low-pass filter
    int result = d->windowAvg();
    // we will print three values each time: 0, 1024, and the measurement
    // the 0 and 1024 simply give us a top and bottom line in the plotter
    Serial.print("0 1024 ");
    Serial.println(result);
}

void setup() {
    // prepare Serial for printing to the plotter
    Serial.begin(2000000);
    // add a printing function as a scheduled callback for our Devlpr
    // here we set the function to be called every 1ms
    devlpr.scheduleFunction(printAvg, 1);
}

void loop() {
    // let the DEVLPR library do its job
    devlpr.tick();
}
