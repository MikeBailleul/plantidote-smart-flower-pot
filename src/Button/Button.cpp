#include "Button.h"

const uint8_t DEBOUNCE_DELAY = 10; // ms

Button::Button(gpio_num_t buttonPin) {
    pin = buttonPin;
    pinMode(pin, INPUT);
}

bool Button::pressed() { return state == 1; }
bool Button::released() { return state == 0xffff; }
bool Button::held(uint16_t count = 0) { return state > 1 + count && state < 0xffff; }

void Button::loopRoutine() {
    bool reading = digitalRead(pin);

    // if the logic level has changed since the last reading,
    // we reset the timer which counts down the necessary time
    // beyond which we can consider that the bouncing effect
    // has passed.
    if (reading != lastReading) {
        lastDebounceTime = millis();
    }

    // from the moment we're out of the bouncing phase
    // the actual status of the button can be determined
    if (millis() - lastDebounceTime > DEBOUNCE_DELAY) {
        bool pressed = reading == HIGH;
        if (pressed) {
                if (state  < 0xfffe) state++;
            else if (state == 0xfffe) state = 2;
        } else if (state) {
            state = state == 0xffff ? 0 : 0xffff;
        }
    }

    // finally, each new reading is saved
    lastReading = reading;
}