#include "Battery.h"

const uint8_t MEASURE_COUNT = 10; // number of measures to do before averaging

// -------------------------------- private methods -------------------------------- //

void Battery::goToNextState() {
    state = (state + 1) % IDLE;
    Serial.println("Go to next battery state: " + String(state));
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
    readingAverage = readingAverage / MEASURE_COUNT;

    // conversion from voltage to percentage by using a fitting function for the charge-voltage-curve of an LiPo battery
    batteryPercentage = 2808.3808 * pow(readingAverage, 4) - 43560.9157 * pow(readingAverage, 3) + 252848.5888 * pow(readingAverage, 2) - 650767.4615 * readingAverage + 626532.9;
    if (readingAverage > maxVoltage) {
        batteryPercentage = 100;
    } else if (readingAverage < minVoltage) {
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

int8_t Battery::getBatteryPercentage() {
    return batteryPercentage;
}
