#ifndef Button_h
#define Button_h

#include <Arduino.h>

class Button {
    private:
        gpio_num_t pin;
        bool lastReading;
        uint32_t lastDebounceTime;
        uint16_t state;

    public:
        Button(gpio_num_t pin);

        bool pressed();
        bool released();
        bool held(uint16_t count);

        void loopRoutine();
};

#endif