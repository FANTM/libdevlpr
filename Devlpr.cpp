#include "Arduino.h"
#include "Devlpr.h"

Devlpr::Devlpr(int pin)
{
    emgPin = pin;
    emgRunningSum = 0;
    bufInd = BUFSIZE - 1;
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

unsigned int Devlpr::lastValue()
{
    return buf[bufInd];
}

unsigned int Devlpr::windowAvg()
{
    return emgRunningSum / BUFSIZE;
}

int Devlpr::lastValueCentered()
{
    int lastVal = lastValue();
    int wAvg = windowAvg();
    return lastVal - wAvg;
}

void Devlpr::readEMG()
{
    // want bufInd to sit on the most recent value until tick
    // so update bufInd before the read
    bufInd++;
    // circular buffer
    if (bufInd >= BUFSIZE) { // faster than mod
        bufInd = 0;
    }
    // read our new value
    emgVal = analogRead(emgPin);
    // before replacing the prev tail value though, update running sum
    emgRunningSum = emgRunningSum - buf[bufInd] + emgVal;
    // now replace
    buf[bufInd] = emgVal;
}
