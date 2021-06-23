#include <libdevlpr.h>

// create a DEVLPR object with the pin we have connected on the shield
// can create multiple for different pins if stacking shields
Devlpr devlpr(A0);

void writeFlex(Devlpr *d) {
    // since this function is called when a flex is detected, just print
    Serial.println("FLEX");
}

void setup() {
    // prepare Serial for printing to the monitor
    Serial.begin(2000000);
    // set the callback for flex detection
    // flex detection is based on a multiple of peak-to-peak amplitude
    // here we set the mutliple to be 1.7x baseline and a cooldown of 300ms
    devlpr.setFlexCallback(writeFlex, 1.7, 300);
}

void loop() {
    // let the DEVLPR library do its job
    devlpr.tick();
}
