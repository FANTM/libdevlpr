#ifndef Devlpr_h
#define Devlpr_h

#include "Arduino.h"

class Devlpr
{
    public:
        Devlpr(int pin=A0);
        void tick();
        unsigned int lastValue();
        int lastValueCentered();
        unsigned int windowAvg();
        unsigned int windowPeakAmplitude();
        unsigned int windowPeakToPeakAmplitude();
        int scheduleFunction(void (*f)(Devlpr *d), unsigned int millisPer);
    private:
        // emg buffer bookkeeping
        static const byte BUFSIZE = 32; // power of 2 for fast integer avg calc
        unsigned int buf[BUFSIZE];
        byte bufInd;
        // emg tracking
        int emgPin;
        unsigned int emgVal; // ATMEGA boards have 10-bit ADC (0-1023)
        unsigned int emgRunningSum; // if BUFSIZE is small, uint is fine
        void readEMG();
        // scheduling
        unsigned long lastTickMicros = 0L;
        // emg scheduling
        static const unsigned long MICROS_SCHED_EMG = 1000L;
        unsigned long microsSinceEMG = 0L;
        // user function scheduling
        static const byte FUNCMAX = 8;
        void (*funcs[FUNCMAX])(Devlpr *d);
        unsigned long schedMicros[FUNCMAX];
        unsigned long microsSince[FUNCMAX];
        byte numFuncs;
};

#endif
