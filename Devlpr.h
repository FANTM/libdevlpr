#ifndef Devlpr_h
#define Devlpr_h

#include "Arduino.h"

class Devlpr
{
    public:
        Devlpr();
        void tick();
        int lastValue();
        float lastValueFiltered();
        float windowAvg();
        float dummy;
    private:
        // general buffer bookkeeping
        static const byte BUFSIZE = 155; // size of filter
        byte bufInd;
        int buf[BUFSIZE];
        unsigned long lastTickMicros = 0;
        // emg tracking
        unsigned long MICROS_SCHED_EMG = 1000;
        unsigned long microsSinceEMG;
        unsigned long emgRunningSum;
        int emgPin;
        int emgVal;
        void readEMG();
};

#endif
