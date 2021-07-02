#include "Arduino.h"
#include "Devlpr.h"

Devlpr::Devlpr(int pin)
{
    emgPin = pin;
    emgRunningSum = 0;
    bufInd = BUFSIZE - 1;
    numFuncs = 0;
    lastFiltVal = 0;
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
    if (microsSinceEMG >= MICROS_SCHED_EMG) {
        readEMG();
        // and update micros since
        microsSinceEMG = 0L;
        // NOTE just a best effort to run on time
    }
    ////////////////
    // Flex Check //
    ////////////////
    // accrue micros on the micros since last check
    microsSinceFlexCheck += microsDelta;
    // check if enough time has passed to check for a flex
    if (microsSinceFlexCheck >= MICROS_SCHED_FLEX) {
        flexCheck(currMicros);
        // and update micros since
        microsSinceFlexCheck = 0L;
    }
    ////////////////////////////
    // User Function Schedule //
    ////////////////////////////
    // go through each function and check if it needs to run
    for (byte i = 0; i < numFuncs; i++) {
        // accrue micros on the micros since last run
        microsSince[i] += microsDelta;
        // check if enough time has passed to run it
        if (microsSince[i] >= schedMicros[i]) {
            // run it
            funcs[i](this);
            // and update micros since
            microsSince[i] = 0L;
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

int Devlpr::lastValueFiltered()
{
    return lastFiltVal;
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

void Devlpr::setFlexCallback(void (*f)(Devlpr *d), float threshMult,
    unsigned int millisCooldown)
{
    // set the callback
    onFlexFunc = f;
    // and the parameters
    flexThreshMultiple = threshMult;
    flexCooldownMicros = millisCooldown * 1000L;
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
    // and we need to calculate the filtered value every sample
    calcFiltered();
}

void Devlpr::flexCheck(unsigned long currMicros)
{
    // to check cooldown
    unsigned long microsDelta = currMicros - prevFlexMicros;
    // the actual flex check
    unsigned int peakToPeakThresh = prevPeakToPeak * flexThreshMultiple;
    unsigned int currPeakToPeak = windowPeakToPeakAmplitude();
    // need an attached function, cooldown passed, and a peak change
    if (onFlexFunc && microsDelta >= flexCooldownMicros &&
        currPeakToPeak >= peakToPeakThresh) {
        // hit the callback
        onFlexFunc(this);
        // and update the previous confirmed flex
        prevFlexMicros = currMicros;
    }
    // need to update the current peak-to-peak for next time
    prevPeakToPeak = currPeakToPeak;
}

void Devlpr::calcFiltered()
{
    // NOTE: IIR filter is recurrent and needs to run every tick to work
    // we need to operate on 0-centered(ish) data and we will be doing float math
    float xn = lastValueCentered();
    // compute the recurrence by section
    for (int s = 0; s < N_SECTIONS; s++) {
        float xn_tmp = xn;
        xn = notch60[s][0] * xn_tmp + z[s][0];
        z[s][0] = (notch60[s][1] * xn_tmp - notch60[s][4] * xn + z[s][1]);
        z[s][1] = (notch60[s][2] * xn_tmp - notch60[s][5] * xn);
    }
    // and store our most recent value for reference later (as an int now)
    lastFiltVal = (int)xn;
}
