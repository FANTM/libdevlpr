#include "Arduino.h"
#include "Devlpr.h"

Devlpr::Devlpr()
{
    bufInd = BUFSIZE - 1;
    emgPin = A5;
    emgRunningSum = 0;
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

int Devlpr::lastValue()
{
    return buf[bufInd];
}

int filt[93] = {24, 23, 100, 109, 130, 128, 81, 68, -39, -41, -181, -140, -280, -170, -294, -109, -225, 14, -124, 134, -61, 184, -87, 136, -205, 14, -365, -115, -491, -176, -523, -132, -450, -4, -320, 140, -215, 218, -204, 181, -306, 43, -479, -128, -637, -244, 9329, -244, -637, -128, -479, 43, -306, 181, -204, 218, -215, 140, -320, -4, -450, -132, -523, -176, -491, -115, -365, 14, -205, 136, -87, 184, -61, 134, -124, 14, -225, -109, -294, -170, -280, -140, -181, -41, -39, 68, 81, 128, 130, 109, 100, 23, 24};
float filtFactor = 10000.0; // the multiplier on the integral filter
float Devlpr::lastValueFiltered()
{
    // should help to roughly recenter data (use quick integral math)
    long sampAvg = emgRunningSum / BUFSIZE;
    // the buffer is the same size as the filter so filtered data
    // will be roughly len/2 ticks in the past
    long ret = 0;
    int currInd = bufInd + 1; // start at the tail
    // the filter is also symmetric, so convolve=linear combination
    for (int i = 0; i < BUFSIZE; i++) {
        // check the current circular buffer index
        if (currInd >= BUFSIZE) { // faster than modulo
            currInd = 0;
        }
        // we will need to coerce the ints to longs to avoid overflow
        long filtVal = (long)filt[i];
        long sampVal = (long)buf[currInd] - sampAvg;
        // do the multiply/add
        ret = ret + (filtVal * sampVal);
        // and update the current buffer index for next
        currInd++;
    }
    // our integral filter is multiplied by some factor, so correct
    return ret / filtFactor;
}

float Devlpr::windowAvg()
{
    return emgRunningSum / float(BUFSIZE);
}

void Devlpr::readEMG()
{
    // want bufInd to sit on the most recent value until tick
    // so update bufInd before the read
    bufInd++;
    // circular buffer
    if (bufInd >= BUFSIZE) { // surprisingly faster than mod
        bufInd = 0;
    }
    // read our new value
    emgVal = analogRead(emgPin);
    // before replacing the prev tail value though, update running sum
    emgRunningSum = emgRunningSum - buf[bufInd] + emgVal;
    // now replace
    buf[bufInd] = emgVal;
}
