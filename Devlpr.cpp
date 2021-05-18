#include "Arduino.h"
#include "Devlpr.h"

Devlpr::Devlpr(int pin)
{
    emgPin = pin;
    emgRunningSum = 0;
    bufInd = BUFSIZE - 1;
    numFuncs = 0;
}

void Devlpr::tick()
{
    // check the current time
    unsigned long currMicros = micros();
    unsigned long microsDelta = currMicros - lastTickMicros;
    //////////////////
    // EMG Schedule //
    //////////////////
    // accrue micros on the micros since last run
    microsSinceEMG += microsDelta;
    // check if enough time has passed to read EMG
    if (microsSinceEMG > MICROS_SCHED_EMG) {
        readEMG();
        // and update micros since
        microsSinceEMG = 0;
        // NOTE just a best effort to run on time
    }
    ////////////////////////////
    // User Function Schedule //
    ////////////////////////////
    // go through each function and check if it needs to run
    for (byte i = 0; i < numFuncs; i++) {
        // accrue micros on the micros since last run
        microsSince[i] += microsDelta;
        // check if enough time has passed to run it
        if (microsSince[i] > schedMicros[i]) {
            // run it
            funcs[i](this);
            // and update micros since
            microsSince[i] = 0;
            // NOTE just a best effort to run on time
        }
    }
    ////////////////
    // Wrap it up //
    ////////////////
    // pretend no time has passed since function start for best effort
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

unsigned int Devlpr::windowPeakAmplitude()
{
    // use the window average as the reference point (should be close to DC offset)
    int wAvg = windowAvg();
    // and need to find the max absolute value from ref
    unsigned int peak = 0;
    for (int i = 0; i < BUFSIZE; i++) { // no need to start from bufInd
        int currDiff = (int)buf[i] - wAvg;
        int currAbs = abs(currDiff);
        if (currAbs > peak) {
            peak = currAbs;
        }
    }
    return peak;
}

unsigned int Devlpr::windowPeakToPeakAmplitude()
{
    // and need to find the max absolute value from ref
    unsigned int peak = 0;
    unsigned int trough = 1023;
    for (int i = 0; i < BUFSIZE; i++) { // no need to start from bufInd
        unsigned int currVal = buf[i];
        if (currVal > peak) {
            peak = currVal;
        }
        if (currVal < trough) {
            trough = currVal;
        }
    }
    return peak - trough;
}

int Devlpr::scheduleFunction(void (*f)(Devlpr *d), unsigned int millisPer)
{
    // check first if we are out of space to attach more functions
    if (numFuncs >= FUNCMAX) {
        return -1;
    }
    // otherwise add it in
    funcs[numFuncs] = f;
    schedMicros[numFuncs] = millisPer * 1000L;
    microsSince[numFuncs] = 0;
    numFuncs++;
    return (numFuncs - 1);
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
