#ifndef Battery_h
#define Battery_h

#include <Arduino.h>

enum BatteryState {
    INITIALIZING = 1,
    MEASURING = 2,
    COMPUTING = 3,
    IDLE = 4
};

class Battery {
    private:
        gpio_num_t pin;
        unsigned long measureWaitingTime = 0;
        float minVoltage;
        float maxVoltage;
        
        uint16_t state;
        unsigned long previousBatteryLevelMeasureTime = 0;
        uint8_t batteryReadingCounter = 0;
        float batteryLevelAverage = 0;
        uint8_t batteryPercentage;
        
        void nextState();
        void init();
        void measure();
        void compute();
        float getVoltage();
        bool isIdle();

    public:
        Battery(gpio_num_t pin, unsigned long measureWaitingTime, float minVoltage, float maxVoltage);

        void loopRoutine();
        void startMeasure();
        uint8_t getBatteryPercentage();
};

#endif