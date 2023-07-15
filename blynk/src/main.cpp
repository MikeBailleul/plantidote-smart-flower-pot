#include <Arduino.h>
#include <Button/Button.h>
#include <Battery/Battery.h>
#include <Moisture/Moisture.h>
#include <WaterPump/WaterPump.h>
#include <WaterLevel/WaterLevel.h>
#include <myconfig.h>

// ------------------------ Blynk import ------------------------ //
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// ------------------------ Pins ---------------------- //

const gpio_num_t PIN_BUTTON = GPIO_NUM_13;

const gpio_num_t PIN_PUMP_POWER = GPIO_NUM_17;
const uint16_t PUMP_DURATION = 5000;

const gpio_num_t PIN_MOISTURE_POWER = GPIO_NUM_19;
const gpio_num_t PIN_MOISTURE_SIGNAL = GPIO_NUM_34;
const uint16_t MOISTURE_CALIBRATION_WATER = 821; // hardcoded for now, TODO claibrate directly from app
const uint16_t MOISTURE_CALIBRATION_AIR = 2535; // hardcoded for now, TODO claibrate directly from app

const gpio_num_t PIN_WATER_POWER_LEVEL_100 = GPIO_NUM_27;
const gpio_num_t PIN_WATER_POWER_LEVEL_70 = GPIO_NUM_26;
const gpio_num_t PIN_WATER_POWER_LEVEL_30 = GPIO_NUM_25;
const gpio_num_t PIN_WATER_POWER_LEVEL_10 = GPIO_NUM_33;
const gpio_num_t PIN_WATER_SIGNAL = GPIO_NUM_32;

const gpio_num_t PIN_BATTERY_LEVEL = GPIO_NUM_35;
const float BATTERY_MIN_VOLTAGE = 3.3;
const float BATTERY_MAX_VOLTAGE = 4.2;

const gpio_num_t PIN_BUILTIN_LED = GPIO_NUM_5;

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

// ------------------------ Declarations ---------------------- //

BlynkTimer blynkTimer;
Button button(PIN_BUTTON);
WaterPump waterPump(PIN_PUMP_POWER, PUMP_DURATION);
Moisture moisture(PIN_MOISTURE_POWER, PIN_MOISTURE_SIGNAL, MEASURE_WAITING_TIME, MOISTURE_CALIBRATION_WATER, MOISTURE_CALIBRATION_AIR);
Battery battery(PIN_BATTERY_LEVEL, MEASURE_WAITING_TIME, BATTERY_MIN_VOLTAGE, BATTERY_MAX_VOLTAGE);
WaterLevel waterLevel(PIN_WATER_SIGNAL, PIN_WATER_POWER_LEVEL_10, PIN_WATER_POWER_LEVEL_30, PIN_WATER_POWER_LEVEL_70, PIN_WATER_POWER_LEVEL_100, MEASURE_WAITING_TIME);

// ------------------------ Blynk Config ---------------------- //

char ssid[] = WIFI_SSID;
char pass[] = WIFI_PASSWORD;

// called every time the device is connected to Blynk.Cloud
BLYNK_CONNECTED() {
}

void sendDataToBlynk() {
    Blynk.virtualWrite(V0, battery.getBatteryVoltage());
    Blynk.virtualWrite(V1, battery.getBatteryPercentage());
    Blynk.virtualWrite(V2, moisture.getMoisturePercentage());
    Blynk.virtualWrite(V3, waterLevel.getWaterPercentage());
}

// ------------------------ Methods ---------------------- //

void waitForNextCycle() {
    uint32_t now;
    do { now = millis(); } while (now - timer.laptime < WAIT_PERIOD);
    timer.laptime = now;
    timer.ticks++;
}

// ------------------------ Core ---------------------- //

void setup() {
    Serial.begin(115200);

    pinMode(PIN_BUILTIN_LED, OUTPUT);

    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
    blynkTimer.setInterval(10000L, sendDataToBlynk);
}

void loop() {
    Blynk.run();
    blynkTimer.run();

    button.loopRoutine();
    battery.loopRoutine();
    moisture.loopRoutine();
    waterPump.loopRoutine();
    waterLevel.loopRoutine();

    if (button.pressed()) {
        Serial.println("---------- Pressed ----------");

        Serial.println("Battery voltage: "+ String(battery.getBatteryVoltage()));
        Serial.println("Battery percentage: "+ String(battery.getBatteryPercentage()));
        Serial.println("Moisture level: "+ String(moisture.getMoisturePercentage()));
        Serial.println("Water level: "+ String(waterLevel.getWaterPercentage()));
        
        battery.startMeasure();
        moisture.startMeasure();
        waterLevel.startMesure();

        digitalWrite(PIN_BUILTIN_LED, !digitalRead(PIN_BUILTIN_LED));

        // waterPump.startPumping();
    }

    if (button.held(50)) {
        Serial.println("Held > 50");
        waterPump.stopPumping();
    }
    
    waitForNextCycle();
}