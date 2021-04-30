#ifndef Devlpr_h
#define Devlpr_h

#include "Arduino.h"

class Devlpr
{
    public:
        Devlpr();
        void tick();
        int lastValue();
    private:
        // general buffer bookkeeping
        static const byte BUFSIZE = 129;
        byte bufInd;
        int buf[BUFSIZE];
        unsigned long lastTickMicros = 0;
        // emg tracking
        unsigned long MICROS_SCHED_EMG = 1000;
        void readEMG();
        int emgPin;
        unsigned long microsSinceEMG;
};

#endif
