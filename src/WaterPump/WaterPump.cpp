#include "WaterPump.h"

// -------------------------------- private methods -------------------------------- //

void WaterPump::goToState(WaterPumpState newState) {
    state = newState % IDLE;
    Serial.println("Force new pump state: " + String(state));
}

void WaterPump::goToNextState() {
    state = (state + 1) % IDLE;
    Serial.println("Go to next water pump state: " + String(state));
}

void WaterPump::initState() {
    pumpOperationStartTime = millis();
    goToNextState();
}

void WaterPump::pumpRunState() {
    if(millis() - pumpOperationStartTime < pumpOperationDuration) {
        digitalWrite(pin, HIGH);
    } else {
        goToNextState();
    }
}

void WaterPump::pumpStopState() {
    digitalWrite(pin, LOW);
    goToNextState();
}

bool WaterPump::isIdle() {
    return state % IDLE == 0;
}

// -------------------------------- public methods -------------------------------- //

WaterPump::WaterPump(gpio_num_t pin, unsigned long pumpOperationDuration) {
    WaterPump::pin = pin;
    WaterPump::pumpOperationDuration = pumpOperationDuration;
    pinMode(pin, OUTPUT);
}

void WaterPump::loopRoutine() {
    switch (state) {
    case IDLE:
        break;

    case INITIALIZING:
        initState();

    case PUMPING:
        pumpRunState();
        break;

    case STOPPING:
        pumpStopState();
        break;
    
    default:
        break;
    }
}

void WaterPump::startPumping() {
    if (isIdle()) {
        goToNextState();
    } else {
        Serial.println("Pump not IDLE, already in state: " + String(state));
    }
}

void WaterPump::stopPumping() {
    if (!isIdle()) {
        goToState(STOPPING);
    } else {
        Serial.println("Pump already IDLE");
    }
}

