#include "WaterLevel.h"

// -------------------------------- private methods -------------------------------- //

void WaterLevel::goToState(WaterLevelState newState) {
    state = newState % IDLE;
    Serial.println("Force new waterLevel state: " + String(state));
}

void WaterLevel::goToNextState() {
    state = (state + 1) % IDLE;
    Serial.println("Go to next waterLevel state: " + String(state));
}

// -------------------------------------------------- //
// 1. level is UNKNOWN
// 2. DISABLE pinPowerLevelLow -> INPUT
//    ENABLE pinPowerLevelGood -> OUTPUT + HIGH
// 3. read pinSignal
//     if pinSignal == HIGH -> level is GOOD -> can go back to IDLE state
//     else 
// 4. DISABLE pinPowerLevelGood -> INPUT
//    ENABLE pinPowerLevelLow -> OUTPUT + HIGH
// 5. read pinSignal
//    if pinSignal == HIGH -> level is LOW
//    else pinSignal == LOW -> level is EMPTY
// -------------------------------------------------- //

void WaterLevel::initState() {
    previousMeasureTime = 0;
    waterReading = LEVEL_UNKNOWN;

    goToNextState();
}

void WaterLevel::enableLevelGoodState() {
    previousMeasureTime = millis();

    pinMode(pinPowerLevelLow, INPUT);
    pinMode(pinPowerLevelGood, OUTPUT);
    digitalWrite(pinPowerLevelGood, HIGH);

    goToNextState();
}

void WaterLevel::measureLevelGoodState() {
    if (millis() - previousMeasureTime >= measureWaitingTime) {
        if (digitalRead(pinSignal) == HIGH) {
            waterReading = LEVEL_GOOD;
            
            goToState(CLEANING);
        } else {
            goToNextState();
        }
    }
}

void WaterLevel::enableLevelLowState() {
    previousMeasureTime = millis();

    pinMode(pinPowerLevelGood, INPUT);
    pinMode(pinPowerLevelLow, OUTPUT);
    digitalWrite(pinPowerLevelLow, HIGH);

    goToNextState();
}

void WaterLevel::measureLevelLowState() {
    if (millis() - previousMeasureTime >= measureWaitingTime) {
        if (digitalRead(pinSignal) == HIGH) {
            waterReading = LEVEL_LOW;
        } else {
            waterReading = LEVEL_EMPTY;  
        }
        
        goToNextState();
    }
}

void WaterLevel::cleaningState() {
    pinMode(pinPowerLevelLow, INPUT);
    pinMode(pinPowerLevelLow, INPUT);
    goToNextState();
}

bool WaterLevel::isIdle() {
    return state % IDLE == 0;
}

// -------------------------------- public methods -------------------------------- //

WaterLevel::WaterLevel(gpio_num_t pinSignal, gpio_num_t pinPowerLevelLow, gpio_num_t pinPowerLevelGood, unsigned long measureWaitingTime) {
    WaterLevel::pinSignal = pinSignal;
    WaterLevel::pinPowerLevelLow = pinPowerLevelLow;
    WaterLevel::pinPowerLevelGood = pinPowerLevelGood;
    WaterLevel::measureWaitingTime = measureWaitingTime;
    pinMode(pinSignal, INPUT);
}

void WaterLevel::loopRoutine() {
    switch (state) {
    case IDLE:
        break;

    case INITIALIZING:
        initState();
        break;

    case ENABLING_LEVEL_GOOD:
        enableLevelGoodState();
        break;

    case MEASURING_LEVEL_GOOD:
        measureLevelGoodState();
        break;

    case ENABLING_LEVEL_LOW:
        enableLevelLowState();
        break;

    case MEASURING_LEVEL_LOW:
        measureLevelLowState();
        break;

    case CLEANING:
        cleaningState();
        break;
    
    default:
        break;
    }
}

void WaterLevel::startMesure() {
    if (isIdle()) {
        goToNextState();
    } else {
        Serial.println("Water level not IDLE, already in state: " + String(state));
    }
}

WaterReading WaterLevel::getWaterReading() {
    return waterReading;
}
