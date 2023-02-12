#ifndef Moisture_h
#define Moisture_h

#include <Arduino.h>

class Moisture {
    private:
        enum MoistureState {
            INITIALIZING = 1,
            MEASURING = 2,
            COMPUTING = 3,
            IDLE = 4
        };

        // ---- constructor parameters ---- //
        gpio_num_t pinPower;
        gpio_num_t pinSignal;
        unsigned long measureWaitingTime = 0;
        uint16_t calibrationWater = 0;
        uint16_t calibrationAir = 0;
        
        // ---- internal variables ---- //
        uint16_t state;
        unsigned long previousMeasureTime = 0;
        uint8_t readingCounter = 0;
        float readingAverage = 0;
        int8_t moisturePercentage = -1;
        
        // ---- state change methods ---- //
        void goToNextState();

        // ---- state execution methods ---- //
        void initState();
        void measureState();
        void computeState();

        // ---- getters ---- //
        bool isIdle();

    public:
        Moisture(gpio_num_t pinPower, gpio_num_t pinSignal, unsigned long measureWaitingTime, uint16_t calibrationWater, uint16_t calibrationAir);

        void loopRoutine();
        void startMeasure();
        int8_t getMoisturePercentage();
};

#endif