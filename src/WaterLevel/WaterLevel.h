#ifndef WaterLevel_h
#define WaterLevel_h

#include <Arduino.h>

enum WaterReading {
    LEVEL_UNKNOWN = -1,
    LEVEL_EMPTY = 0,
    LEVEL_LOW = 1,
    LEVEL_GOOD = 2
};

class WaterLevel {
    private:
        enum WaterLevelState {
            INITIALIZING = 1,
            ENABLING_LEVEL_GOOD = 2,
            MEASURING_LEVEL_GOOD = 3,
            ENABLING_LEVEL_LOW = 4,
            MEASURING_LEVEL_LOW = 5,
            CLEANING = 6,
            IDLE = 7
        };

        // ---- constructor parameters ---- //
        gpio_num_t pinSignal;
        gpio_num_t pinPowerLevelLow;
        gpio_num_t pinPowerLevelGood;
        unsigned long measureWaitingTime = 0;
        
        // ---- internal variables ---- //
        uint16_t state;
        unsigned long previousMeasureTime = 0;
        WaterReading waterReading = LEVEL_UNKNOWN;
        
        // ---- state change methods ---- //
        void goToState(WaterLevelState state);
        void goToNextState();

        // ---- state execution methods ---- //
        void initState();
        void enableLevelGoodState();
        void measureLevelGoodState();
        void enableLevelLowState();
        void measureLevelLowState();
        void cleaningState();
        void idleState();

        // ---- getters ---- //
        bool isIdle();

    public:
        WaterLevel(gpio_num_t pinSignal, gpio_num_t pinPowerLevelLow, gpio_num_t pinPowerLevelGood, unsigned long measureWaitingTime);

        void loopRoutine();
        void startMesure();
        WaterReading getWaterReading();
};

#endif