#include "Arduino.h"
#include "Devlpr.h"

Devlpr::Devlpr(int pin, int filterType)
{
    emgPin = pin;
    rawEmgRunningSum = 0;
    bufInd = BUFSIZE - 1;
    numFuncs = 0;
    if (filterType != FILTER_NONE) {
        doFilter = true;
        initFilter(filterType);
    }
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

int Devlpr::lastValue(bool filtered)
{
    if (filtered) {
        return filterBuf[bufInd];
    }
    return buf[bufInd];
}

int Devlpr::windowAvg(bool filtered)
{
    // int math should be good enough
    if (filtered) {
        return filterEmgRunningSum / BUFSIZE;
    }
    return rawEmgRunningSum / BUFSIZE;
}

int Devlpr::lastValueCentered(bool filtered)
{
    int lastVal = lastValue(filtered);
    int wAvg = windowAvg(filtered);
    return lastVal - wAvg;
}

int Devlpr::windowPeakAmplitude(bool filtered)
{
    // use the window average as the reference point (should be close to DC offset)
    int wAvg = windowAvg(filtered);
    // and need to find the max absolute value from ref
    int peak = 0;
    // hear me out, I'm duplicating the loop to not determine the buffer each iteration
    // if it helps, I feel just awful about it
    if (filtered) {
        for (int i = 0; i < BUFSIZE; i++) { // no need to start from bufInd
            int currDiff = filterBuf[i] - wAvg;
            int currAbs = abs(currDiff);
            if (currAbs > peak) {
                peak = currAbs;
            }
        }
    }
    else {
        for (int i = 0; i < BUFSIZE; i++) { // no need to start from bufInd
            int currDiff = buf[i] - wAvg;
            int currAbs = abs(currDiff);
            if (currAbs > peak) {
                peak = currAbs;
            }
        }
    }
    return peak;
}

int Devlpr::windowPeakToPeakAmplitude(bool filtered)
{
    // and need to find the max absolute value from ref
    int peak = 0;
    int trough = 1023;
    // hear me out, I'm duplicating the loop to not determine the buffer each iteration
    // if it helps, I feel just awful about it
    if (filtered) {
        for (int i = 0; i < BUFSIZE; i++) { // no need to start from bufInd
            int currVal = filterBuf[i];
            if (currVal > peak) {
                peak = currVal;
            }
            if (currVal < trough) {
                trough = currVal;
            }
        }
    }
    else {
        for (int i = 0; i < BUFSIZE; i++) { // no need to start from bufInd
            int currVal = buf[i];
            if (currVal > peak) {
                peak = currVal;
            }
            if (currVal < trough) {
                trough = currVal;
            }
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
    // read our new raw value
    emgVal = analogRead(emgPin);
    // before replacing the prev tail value though, update running sum
    rawEmgRunningSum = rawEmgRunningSum - buf[bufInd];
    rawEmgRunningSum = rawEmgRunningSum + emgVal;
    // now replace
    buf[bufInd] = emgVal;
    // and handle all the filtering if set
    if (doFilter) {
        handleFiltered();
    }
}

void Devlpr::flexCheck(unsigned long currMicros)
{
    // to check cooldown
    unsigned long microsDelta = currMicros - prevFlexMicros;
    // the actual flex check
    int peakToPeakThresh = prevPeakToPeak * flexThreshMultiple;
    // if filter is configured, base it on filtered values
    int currPeakToPeak = windowPeakToPeakAmplitude(doFilter);
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

void Devlpr::handleFiltered()
{
    // NOTE: IIR filter is recurrent and needs to run every tick to work
    // we need to operate on 0-centered(ish) data and we will be doing float math
    float xn = lastValueCentered(false); // needs to be based on raw data
    // compute the recurrence by section
    for (int s = 0; s < N_SECTIONS; s++) {
        float xn_tmp = xn;
        xn = filter[s][0] * xn_tmp + z[s][0];
        z[s][0] = (filter[s][1] * xn_tmp - filter[s][4] * xn + z[s][1]);
        z[s][1] = (filter[s][2] * xn_tmp - filter[s][5] * xn);
    }
    // and store our filtered value for reference later (as an int now)
    int filtVal = (int)xn;
    // before replacing the prev tail value though, update running sum
    filterEmgRunningSum = filterEmgRunningSum - filterBuf[bufInd];
    filterEmgRunningSum = filterEmgRunningSum + filtVal;
    // now replace
    filterBuf[bufInd] = filtVal;
}

void Devlpr::initFilter(int filterType) {
    // NOTE: this is just a dumb way to not have to reference a different
    // filter array depending on the filter type selected at the start
    
    // 2nd order Butterworth notch for 50Hz
    // {{0.95654323, -1.82035157, 0.95654323, 1., -1.84458768, 0.9536256},
    // { 1.        , -1.90305207, 1.        , 1., -1.87701816, 0.95947072}}
    // 2nd order Butterworth notch for 60Hz
    // {{0.95654323, -1.77962093, 0.95654323, 1., -1.80093517, 0.95415195},
    // { 1.        , -1.860471  , 1.        , 1., -1.83739919, 0.95894143}}
    if (filterType == FILTER_50HZ) {
        filter[0][0] = 0.95654323;
        filter[0][1] = -1.82035157;
        filter[0][2] = 0.95654323;
        filter[0][3] = 1.;
        filter[0][4] = -1.84458768;
        filter[0][5] = 0.9536256;
        filter[1][0] = 1.;
        filter[1][1] = -1.90305207;
        filter[1][2] = 1.;
        filter[1][3] = 1.;
        filter[1][4] = -1.87701816;
        filter[1][5] = 0.95947072;
    }
    if (filterType == FILTER_60HZ) {
        filter[0][0] = 0.95654323;
        filter[0][1] = -1.77962093;
        filter[0][2] = 0.95654323;
        filter[0][3] = 1.;
        filter[0][4] = -1.80093517;
        filter[0][5] = 0.95415195;
        filter[1][0] = 1.;
        filter[1][1] = -1.860471;
        filter[1][2] = 1.;
        filter[1][3] = 1.;
        filter[1][4] = -1.83739919;
        filter[1][5] = 0.95894143;
    }
}
