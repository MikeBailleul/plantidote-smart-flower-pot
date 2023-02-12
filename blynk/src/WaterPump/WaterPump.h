#ifndef WaterPump_h
#define WaterPump_h

#include <Arduino.h>

class WaterPump {
    private:
        enum WaterPumpState {
            INITIALIZING = 1,
            PUMPING = 2,
            STOPPING = 3,
            IDLE = 4
        };

        // ---- constructor parameters ---- //
        gpio_num_t pin;
        unsigned long pumpOperationDuration;

        // ---- internal variables ---- //
        uint16_t state;
        unsigned long pumpOperationStartTime;
        
        // ---- state change methods ---- //
        void goToState(WaterPumpState state);
        void goToNextState();

        // ---- state execution methods ---- //
        void initState();
        void pumpRunState();
        void pumpStopState();

        // ---- getters ---- //
        bool isIdle();

    public:
        WaterPump(gpio_num_t pin, unsigned long pumpOperationDuration);

        void loopRoutine();
        void startPumping();
        void stopPumping();
};

#endif