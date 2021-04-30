#include "Arduino.h"
#include "Devlpr.h"

Devlpr::Devlpr()
{
    bufInd = 0;
    emgPin = A0;
}

void Devlpr::tick()
{
    // check the current time
    unsigned long currMicros = micros();
    unsigned long microsDelta = currMicros - lastTickMicros;
    // go through each function and see if we need to run it
    // accrue micros on the micros since last run
    microsSinceEMG += microsDelta;
    // now see if enough time has passed to run this bad boy
    // keep in mind that the schedule is in millis vs micros
    if (microsSinceEMG > MICROS_SCHED_EMG) {
        readEMG();
        // and update micros since
        microsSinceEMG = 0;
        // NOTE do we want to do some sort of remainder on
        // NOTE the micros - ie do we want to play catch up
        // NOTE or just make a best effort to run on sched?
    }
    // just pretend no time has passed since function start
    lastTickMicros = currMicros;
}

int Devlpr::lastValue()
{
    return buf[bufInd];
}

void Devlpr::readEMG()
{
    buf[bufInd] = analogRead(emgPin);
    Serial.println(buf[bufInd]);
    bufInd++;
    // circular buffer
    if (bufInd > BUFSIZE) {
        bufInd = 0;
    }
}
