#include "Battery.h"

const uint8_t MEASURE_COUNT = 10; // number of measures to do before averaging

// -------------------------------- private methods -------------------------------- //

void Battery::nextState() {
    state = (state + 1) % IDLE;
    Serial.println("New battery state: " + String(state));
}

void Battery::init() {
    batteryLevelAverage = 0;
    batteryReadingCounter = 0;
    previousBatteryLevelMeasureTime = 0;
    nextState();
}

void Battery::measure() {
    if(batteryReadingCounter < MEASURE_COUNT && millis() - previousBatteryLevelMeasureTime >= measureWaitingTime) {
        previousBatteryLevelMeasureTime = millis();
        float batteryVoltage = getVoltage();
        batteryLevelAverage += batteryVoltage;
        batteryReadingCounter++;
    }

    if(batteryReadingCounter == MEASURE_COUNT) {
        nextState();
    }
}

void Battery::compute() {
    batteryLevelAverage = batteryLevelAverage / MEASURE_COUNT;

    // conversion from voltage to percentage by using a fitting function for the charge-voltage-curve of an LiPo battery
    batteryPercentage = 2808.3808 * pow(batteryLevelAverage, 4) - 43560.9157 * pow(batteryLevelAverage, 3) + 252848.5888 * pow(batteryLevelAverage, 2) - 650767.4615 * batteryLevelAverage + 626532.9;
    if (batteryLevelAverage > maxVoltage) {
        batteryPercentage = 100;
    } else if (batteryLevelAverage < minVoltage) {
        batteryPercentage = 0;
    }

    nextState();
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
        init();
        break;

    case MEASURING:
        measure();
        break;

    case COMPUTING:
        compute();
        break;
    
    default:
        break;
    }
}

void Battery::startMeasure() {
    if (isIdle()) {
        nextState();
    } else {
        Serial.println("Battery not IDLE, already in state: " + String(state));
    }
}

uint8_t Battery::getBatteryPercentage() {
    return batteryPercentage;
}
