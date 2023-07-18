#include "WaterLevel.h"

const uint8_t MEASURE_COUNT = 10; // number of measures to do before averaging

// -------------------------------- private methods -------------------------------- //

void WaterLevel::goToState(WaterLevelState newState) {
    state = newState % IDLE;
    // Serial.println("Force new waterLevel state: " + String(state));
}

void WaterLevel::goToNextState() {
    state = (state + 1) % IDLE;
    // Serial.println("Go to next waterLevel state: " + String(state));
}

void WaterLevel::initState() {
    previousMeasureTime = 0;
    waterReading = LEVEL_UNKNOWN;
    readingHighest = 0;
    readingCounter = 0;

    goToNextState();
}

void WaterLevel::enableLevelState(WaterReading waterLevel) {
    previousMeasureTime = millis();

    switch (waterLevel)
    {
        case LEVEL_100:
            // disable unwanted pins
            pinMode(pinPowerLevel10, INPUT);
            pinMode(pinPowerLevel30, INPUT);
            pinMode(pinPowerLevel70, INPUT);
            // enable wanted pin
            pinMode(pinPowerLevel100, OUTPUT);
            digitalWrite(pinPowerLevel100, HIGH);

            break;

        case LEVEL_70:
            // disable unwanted pins
            pinMode(pinPowerLevel10, INPUT);
            pinMode(pinPowerLevel30, INPUT);
            pinMode(pinPowerLevel100, INPUT);
            // enable wanted pin
            pinMode(pinPowerLevel70, OUTPUT);
            digitalWrite(pinPowerLevel70, HIGH);

            break;

        case LEVEL_30:
            // disable unwanted pins
            pinMode(pinPowerLevel10, INPUT);
            pinMode(pinPowerLevel70, INPUT);
            pinMode(pinPowerLevel100, INPUT);
            // enable wanted pin
            pinMode(pinPowerLevel30, OUTPUT);
            digitalWrite(pinPowerLevel30, HIGH);

            break;

        case LEVEL_10:
            // disable unwanted pins
            pinMode(pinPowerLevel30, INPUT);
            pinMode(pinPowerLevel70, INPUT);
            pinMode(pinPowerLevel100, INPUT);
            // enable wanted pin
            pinMode(pinPowerLevel10, OUTPUT);
            digitalWrite(pinPowerLevel10, HIGH);

            break;
    
        default:
            break;
    }
    
    goToNextState();
}

void WaterLevel::measureLevelState(WaterReading waterLevel) {
    if (readingCounter < MEASURE_COUNT && millis() - previousMeasureTime >= measureWaitingTime) {
        previousMeasureTime = millis();

        float voltage = analogRead(pinSignal) * (3.0 / 1023.0);
        readingHighest = max(voltage, readingHighest);
        readingCounter++;
    }

    if (readingCounter == MEASURE_COUNT) {
        readingCounter = 0;
        if (readingHighest > 0.1) {
            waterReading = waterLevel;  
            goToState(CLEANING);
        } else {
            if (waterLevel == LEVEL_10) {
                waterReading = LEVEL_EMPTY;  
            }
            goToNextState();
        }
    }
}

void WaterLevel::cleaningState() {
    pinMode(pinPowerLevel100, INPUT);
    pinMode(pinPowerLevel70, INPUT);
    pinMode(pinPowerLevel30, INPUT);
    pinMode(pinPowerLevel10, INPUT);
    goToNextState();
}

bool WaterLevel::isIdle() {
    return state % IDLE == 0;
}

// -------------------------------- public methods -------------------------------- //

WaterLevel::WaterLevel(gpio_num_t pinSignal, gpio_num_t pinPowerLevel10, gpio_num_t pinPowerLevel30, gpio_num_t pinPowerLevel70, gpio_num_t pinPowerLevel100, unsigned long measureWaitingTime) {
    WaterLevel::pinSignal = pinSignal;
    WaterLevel::pinPowerLevel10 = pinPowerLevel10;
    WaterLevel::pinPowerLevel30 = pinPowerLevel30;
    WaterLevel::pinPowerLevel70 = pinPowerLevel70;
    WaterLevel::pinPowerLevel100 = pinPowerLevel100;
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

    case ENABLING_LEVEL_100:
        enableLevelState(LEVEL_100);
        break;

    case MEASURING_LEVEL_100:
        measureLevelState(LEVEL_100);
        break;

    case ENABLING_LEVEL_70:
        enableLevelState(LEVEL_70);
        break;

    case MEASURING_LEVEL_70:
        measureLevelState(LEVEL_70);
        break;

    case ENABLING_LEVEL_30:
        enableLevelState(LEVEL_30);
        break;

    case MEASURING_LEVEL_30:
        measureLevelState(LEVEL_30);
        break;

    case ENABLING_LEVEL_10:
        enableLevelState(LEVEL_10);
        break;

    case MEASURING_LEVEL_10:
        measureLevelState(LEVEL_10);
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

WaterReading WaterLevel::getWaterPercentage() {
    return waterReading;
}
