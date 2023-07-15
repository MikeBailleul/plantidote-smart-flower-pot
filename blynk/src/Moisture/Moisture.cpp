#include "Moisture.h"

const uint8_t MEASURE_COUNT = 10; // number of measures to do before averaging

// -------------------------------- private methods -------------------------------- //

void Moisture::goToNextState() {
    state = (state + 1) % IDLE;
    // Serial.println("Go to next moisture state: " + String(state));
}

void Moisture::initState() {
    readingAverage = 0;
    readingCounter = 0;
    previousMeasureTime = 0;
    digitalWrite(pinPower, HIGH);
    goToNextState();
}

void Moisture::measureState() {
    if (readingCounter < MEASURE_COUNT && millis() - previousMeasureTime >= measureWaitingTime) {
        previousMeasureTime = millis();
        float moistureReading = analogRead(pinSignal);
        readingAverage += moistureReading;
        readingCounter++;
    }

    if (readingCounter == MEASURE_COUNT) {
        digitalWrite(pinPower, LOW);
        goToNextState();
    }
}

void Moisture::computeState() {
    readingAverage = readingAverage / MEASURE_COUNT;
    
    uint16_t moistureCalibrated = map(readingAverage, calibrationWater, calibrationAir, 1320, 3173);
     // conversion from range to percentage by using a fitting function for capacitive moisture sensor
    int percentage = (178147020.5 - 52879.727 * moistureCalibrated) / (1 - 428.814 * moistureCalibrated + 0.9414 * pow(moistureCalibrated, 2));

    if (percentage > 100) {
        moisturePercentage = 100; 
    } else if (percentage < 0) {
        moisturePercentage = 0; 
    } else {
        moisturePercentage = percentage;
    }

    goToNextState();
}

bool Moisture::isIdle() {
    return state % IDLE == 0;
}

// -------------------------------- public methods -------------------------------- //

Moisture::Moisture(gpio_num_t pinPower, gpio_num_t pinSignal, unsigned long measureWaitingTime, uint16_t calibrationWater, uint16_t calibrationAir) {
    Moisture::pinPower = pinPower;
    Moisture::pinSignal = pinSignal;
    Moisture::measureWaitingTime = measureWaitingTime;
    Moisture::calibrationWater = calibrationWater;
    Moisture::calibrationAir = calibrationAir;
    pinMode(pinPower, OUTPUT);
    pinMode(pinSignal, INPUT);
}

void Moisture::loopRoutine() {
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

void Moisture::startMeasure() {
    if (isIdle()) {
        goToNextState();
    } else {
        Serial.println("Moisture not IDLE, already in state: " + String(state));
    }
}

int8_t Moisture::getMoisturePercentage() {
    return moisturePercentage;
}
