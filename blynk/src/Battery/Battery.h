#ifndef Battery_h
#define Battery_h

#include <Arduino.h>

class Battery {
    private:
        enum BatteryState {
            INITIALIZING = 1,
            MEASURING = 2,
            COMPUTING = 3,
            IDLE = 4
        };

        // ---- constructor parameters ---- //
        gpio_num_t pin;
        unsigned long measureWaitingTime = 0;
        float minVoltage;
        float maxVoltage;
        
        // ---- internal variables ---- //
        uint16_t state;
        unsigned long previousMeasureTime = 0;
        uint8_t readingCounter = 0;
        float readingAverage = 0;
        float batteryVoltage = 0.0;
        int8_t batteryPercentage = -1;
        
        // ---- state change methods ---- //
        void goToNextState();

        // ---- state execution methods ---- //
        void initState();
        void measureState();
        void computeState();

        // ---- getters ---- //
        float getVoltage();
        bool isIdle();

    public:
        Battery(gpio_num_t pin, unsigned long measureWaitingTime, float minVoltage, float maxVoltage);

        void loopRoutine();
        void startMeasure();
        float getBatteryVoltage();
        int8_t getBatteryPercentage();
};

#endif