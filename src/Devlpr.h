#ifndef Devlpr_h
#define Devlpr_h

#include "Arduino.h"

#define FILTER_NONE 0
#define FILTER_50HZ 1
#define FILTER_60HZ 2

class Devlpr
{
    public:
        Devlpr(int pin=A0, int filterType=FILTER_NONE);
        void tick();
        int lastValue(bool filtered=false);
        int lastValueCentered(bool filtered=false);
        int windowAvg(bool filtered=false);
        int windowPeakAmplitude(bool filtered=false);
        int windowPeakToPeakAmplitude(bool filtered=false);
        int scheduleFunction(void (*f)(Devlpr *d), unsigned int millisPer);
        void setFlexCallback(void (*f)(Devlpr *d), float threshMult=1.5,
            unsigned int millisCooldown=400);
    private:
        // emg buffer bookkeeping
        static const byte BUFSIZE = 32; // power of 2 for fast integer avg calc
        int buf[BUFSIZE];
        byte bufInd;
        // emg tracking
        int emgPin;
        int emgVal; // ATMEGA boards have 10-bit ADC (0-1023)
        int rawEmgRunningSum; // if BUFSIZE is <= 32, int is fine (max sum=32*1023)
        void readEMG();
        // emg filtering
        void handleFiltered();
        void initFilter(int filterType); // this is dumb
        bool doFilter = false;
        static const byte N_SECTIONS = 2; // 2nd order
        float filter[2][6];
        float z[2][2] = { // maintain recurrent state
            {0.0, 0.0},
            {0.0, 0.0}
        };
        int filterBuf[BUFSIZE]; // need to keep a separate buffer for filtered values
        int filterEmgRunningSum; // if BUFSIZE is <= 32, int is fine
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
