#ifndef WaterLevel_h
#define WaterLevel_h

#include <Arduino.h>

enum WaterReading {
    LEVEL_UNKNOWN = -1,
    LEVEL_EMPTY = 0,
    LEVEL_10 = 10,
    LEVEL_30 = 30,
    LEVEL_70 = 70,
    LEVEL_100 = 100
};

class WaterLevel {
    private:
        enum WaterLevelState {
            INITIALIZING = 1,
            ENABLING_LEVEL_100 = 2,
            MEASURING_LEVEL_100 = 3,
            ENABLING_LEVEL_70 = 4,
            MEASURING_LEVEL_70 = 5,
            ENABLING_LEVEL_30 = 6,
            MEASURING_LEVEL_30 = 7,
            ENABLING_LEVEL_10 = 8,
            MEASURING_LEVEL_10 = 9,
            CLEANING = 10,
            IDLE = 11
        };

        // ---- constructor parameters ---- //
        gpio_num_t pinSignal;
        gpio_num_t pinPowerLevel10;
        gpio_num_t pinPowerLevel30;
        gpio_num_t pinPowerLevel70;
        gpio_num_t pinPowerLevel100;
        unsigned long measureWaitingTime = 0;
        
        // ---- internal variables ---- //
        uint16_t state;
        unsigned long previousMeasureTime = 0;
        uint8_t readingCounter = 0;
        float readingHighest = 0;
        WaterReading waterReading = LEVEL_EMPTY;
        
        // ---- state change methods ---- //
        void goToState(WaterLevelState state);
        void goToNextState();

        // ---- state execution methods ---- //
        void initState();
        void enableLevelState(WaterReading waterLevel);
        void measureLevelState(WaterReading waterLevel);
        void cleaningState();
        void idleState();

        // ---- getters ---- //
        bool isIdle();

    public:
        WaterLevel(gpio_num_t pinSignal, gpio_num_t pinPowerLevel10, gpio_num_t pinPowerLevel30, gpio_num_t pinPowerLevel70, gpio_num_t pinPowerLevel100, unsigned long measureWaitingTime);

        void loopRoutine();
        void startMesure();
        WaterReading getWaterPercentage();
};

#endif