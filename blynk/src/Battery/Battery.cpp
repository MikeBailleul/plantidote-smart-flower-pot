#include "Battery.h"

const uint8_t MEASURE_COUNT = 10; // number of measures to do before averaging

// -------------------------------- private methods -------------------------------- //

void Battery::goToNextState() {
    state = (state + 1) % IDLE;
    // Serial.println("Go to next battery state: " + String(state));
}

void Battery::initState() {
    readingAverage = 0;
    readingCounter = 0;
    previousMeasureTime = 0;
    goToNextState();
}

void Battery::measureState() {
    if (readingCounter < MEASURE_COUNT && millis() - previousMeasureTime >= measureWaitingTime) {
        previousMeasureTime = millis();
        float batteryVoltage = getVoltage();
        readingAverage += batteryVoltage;
        readingCounter++;
    }

    if (readingCounter == MEASURE_COUNT) {
        goToNextState();
    }
}

void Battery::computeState() {
    batteryVoltage = readingAverage / MEASURE_COUNT;

    batteryPercentage = (batteryVoltage - minVoltage) / (maxVoltage - minVoltage) * 100;
    if (batteryVoltage > maxVoltage) {
        batteryPercentage = 100;
    } else if (batteryVoltage < minVoltage) {
        batteryPercentage = 0;
    }

    goToNextState();
}

float Battery::getVoltage() {
    // https://youtu.be/yZjpYmWVLh8?t=158
    return analogRead(pin) / 4096.0 * 7.445;
}

bool Battery::isIdle() {
    return state % IDLE == 0;
}

// -------------------------------- public methods -------------------------------- //

Battery::Battery(gpio_num_t pin, unsigned long measureWaitingTime, float minVoltage, float maxVoltage) {
    Battery::pin = pin;
    Battery::measureWaitingTime = measureWaitingTime;
    Battery::minVoltage = minVoltage;
    Battery::maxVoltage = maxVoltage;
    pinMode(pin, OUTPUT);
}

void Battery::loopRoutine() {
    switch (state) {
    case IDLE:
        break;

    case INITIALIZING:
        initState();
        break;

    case MEASURING:
        measureState();
        break;

    case COMPUTING:
        computeState();
        break;
    
    default:
        break;
    }
}

void Battery::startMeasure() {
    if (isIdle()) {
        goToNextState();
    } else {
        Serial.println("Battery not IDLE, already in state: " + String(state));
    }
}

float Battery::getBatteryVoltage() {
    return batteryVoltage;
}

int8_t Battery::getBatteryPercentage() {
    return batteryPercentage;
}
