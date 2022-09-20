#include <Arduino.h>
#include <Button/Button.h>
#include <Battery/Battery.h>
#include <WaterPump/WaterPump.h>
#include <WaterLevel/WaterLevel.h>

// ------------------------ Pins ---------------------- //

const gpio_num_t PIN_BUTTON = GPIO_NUM_0;
const gpio_num_t PIN_PUMP_POWER = GPIO_NUM_32;

const gpio_num_t PIN_MOISTURE_POWER = GPIO_NUM_19;
const gpio_num_t PIN_MOISTURE_SIGNAL = GPIO_NUM_33;
const gpio_num_t PIN_WATER_SIGNAL = GPIO_NUM_25;
const gpio_num_t PIN_WATER_POWER_LEVEL_LOW = GPIO_NUM_26;
const gpio_num_t PIN_WATER_POWER_LEVEL_GOOD = GPIO_NUM_27;


const gpio_num_t PIN_BATTERY_LEVEL = GPIO_NUM_35;
const float BATTERY_MIN_VOLTAGE = 3.5;
const float BATTERY_MAX_VOLTAGE = 4.2;

// ------------------------ Constants ---------------------- //

const uint16_t MEASURE_WAITING_TIME = 250; // time to wait between measurements

// ------------------------ Cycle controller ---------------------- //

const uint16_t LOOP_FREQUENCY = 25; // Hz
const uint16_t WAIT_PERIOD = 1000 / LOOP_FREQUENCY; // ms
struct Timer {
    uint32_t laptime;
    uint32_t ticks;
};
Timer timer;

void waitForNextCycle() {
    uint32_t now;
    do { now = millis(); } while (now - timer.laptime < WAIT_PERIOD);
    timer.laptime = now;
    timer.ticks++;
}

// ------------------------ Declarations ---------------------- //

Button button(PIN_BUTTON);
Battery battery(PIN_BATTERY_LEVEL, MEASURE_WAITING_TIME, BATTERY_MIN_VOLTAGE, BATTERY_MAX_VOLTAGE);
WaterPump waterPump(PIN_PUMP_POWER, 5000);
WaterLevel waterLevel(PIN_WATER_SIGNAL, PIN_WATER_POWER_LEVEL_LOW, PIN_WATER_POWER_LEVEL_GOOD, MEASURE_WAITING_TIME);

// ------------------------ Methods ---------------------- //

void setup() {
    Serial.begin(115200);

    pinMode(PIN_MOISTURE_SIGNAL, INPUT);
    pinMode(PIN_MOISTURE_POWER, OUTPUT);
}

void loop() {
    button.loopRoutine();
    battery.loopRoutine();
    waterPump.loopRoutine();
    waterLevel.loopRoutine();

    if (button.pressed()) {
        Serial.println("Pressed");

        Serial.println("Battery percentage: "+ String(battery.getBatteryPercentage()));
        Serial.println("Water level: "+ String(waterLevel.getWaterReading()));
        
        battery.startMeasure();
        waterLevel.startReading();
        waterPump.startPumping();
    }

    if (button.held(50)) {
        Serial.println("Held > 50");
        waterPump.stopPumping();
    }

    waitForNextCycle();
}