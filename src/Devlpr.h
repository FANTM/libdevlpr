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
        void setFlexCallback(void (*f)(Devlpr *d), float threshMult=1.5,
            unsigned int millisCooldown=400);
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
        // onFlex scheduling
        static const unsigned long MICROS_SCHED_FLEX = 1000L * BUFSIZE; // once per buffer fill
        unsigned long microsSinceFlexCheck = 0L;
        // flex tracking
        void (*onFlexFunc)(Devlpr *d) = NULL;
        void flexCheck(unsigned long currMicros);
        unsigned long prevFlexMicros = 0L;
        unsigned int prevPeakToPeak = 0L;
        unsigned long flexCooldownMicros = 400000L; // 400ms
        float flexThreshMultiple = 1.5;
        // user function scheduling
        static const byte FUNCMAX = 8;
        void (*funcs[FUNCMAX])(Devlpr *d);
        unsigned long schedMicros[FUNCMAX];
        unsigned long microsSince[FUNCMAX];
        byte numFuncs;
};

#endif
