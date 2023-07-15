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

// ------------------------ Deep sleep config ---------------------- //

#define WAKEUP_LEVEL  HIGH  // Trigger wakeup when button is HIGH
#define uS_TO_S_FACTOR 1000000LL  // Conversion factor for micro seconds to seconds
#define TIME_TO_SLEEP  86400 * uS_TO_S_FACTOR
#define TIME_TO_SLEEP_FOREVER  31536000 * uS_TO_S_FACTOR

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
const float BATTERY_THRESHOLD = 3.4;

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
Timer cycleTimer;
BlynkTimer pumpTimer;
BlynkTimer hibernationTimer;

// ------------------------ Declarations ---------------------- //
Button button(PIN_BUTTON);
WaterPump waterPump(PIN_PUMP_POWER, PUMP_DURATION);
Moisture moisture(PIN_MOISTURE_POWER, PIN_MOISTURE_SIGNAL, MEASURE_WAITING_TIME, MOISTURE_CALIBRATION_WATER, MOISTURE_CALIBRATION_AIR);
Battery battery(PIN_BATTERY_LEVEL, MEASURE_WAITING_TIME, BATTERY_MIN_VOLTAGE, BATTERY_MAX_VOLTAGE);
WaterLevel waterLevel(PIN_WATER_SIGNAL, PIN_WATER_POWER_LEVEL_10, PIN_WATER_POWER_LEVEL_30, PIN_WATER_POWER_LEVEL_70, PIN_WATER_POWER_LEVEL_100, MEASURE_WAITING_TIME);

int8_t moistureThreshold = 50;

// ------------------------ Blynk Config ---------------------- //

char ssid[] = WIFI_SSID;
char pass[] = WIFI_PASSWORD;

// called every time the device is connected to Blynk.Cloud
BLYNK_CONNECTED() {
}

// called every time the V4 state changes
BLYNK_WRITE(V4) {
    int v4Value = param.asInt();

    if (v4Value < 0) {
        moistureThreshold = 0;
    } else if (v4Value > 100) {
        moistureThreshold = 100;
    } else {
        moistureThreshold = v4Value;
    }

    Serial.println("V4 received, moisture threshold: " + String(moistureThreshold));
}

void turnOffBlynkPumpSwitch() {
    Blynk.virtualWrite(V5, 0);
}

// called every time the V5 state changes
BLYNK_WRITE(V5) {
    int v5Value = param.asInt();

    if (v5Value == 1) {
        waterPump.startPumping();
        Blynk.virtualWrite(V6, 1); // log pump action
        pumpTimer.setTimeout(PUMP_DURATION, turnOffBlynkPumpSwitch);
    } else {
        waterPump.stopPumping();
    }

    Serial.println("V5 received, switch pumping ON/OFF: " + String(v5Value));
}

void sendDataToBlynk() {
    Serial.println("Sending data to Blynk");

    Serial.println("Battery voltage: "+ String(battery.getBatteryVoltage()));
    Serial.println("Battery percentage: "+ String(battery.getBatteryPercentage()));
    Serial.println("Moisture level: "+ String(moisture.getMoisturePercentage()));
    Serial.println("Water level: "+ String(waterLevel.getWaterPercentage()));

    Blynk.virtualWrite(V0, battery.getBatteryVoltage());
    Blynk.virtualWrite(V1, battery.getBatteryPercentage());
    Blynk.virtualWrite(V2, moisture.getMoisturePercentage());
    Blynk.virtualWrite(V3, waterLevel.getWaterPercentage());
}

// ------------------------ Deep sleep functions ---------------------- //

void deepSleep(long long int timeToSleep) {
    esp_sleep_enable_ext0_wakeup((gpio_num_t)PIN_BUTTON, WAKEUP_LEVEL);
    esp_sleep_enable_timer_wakeup(timeToSleep);
    esp_deep_sleep_start();
}


void hibernate() {
    Serial.println("--- ZZZZZzzzzz ---");

    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_XTAL,         ESP_PD_OPTION_OFF);
    deepSleep(TIME_TO_SLEEP);
}

void sleepForeverIfNeeded() {
    if (battery.getBatteryVoltage() <= BATTERY_THRESHOLD) {
        Blynk.virtualWrite(V7, 0);
        Serial.println("--- Sleep forever because battery is lower than safety threshold ---");

        esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
        esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
        esp_sleep_pd_config(ESP_PD_DOMAIN_XTAL,         ESP_PD_OPTION_OFF);

        deepSleep(TIME_TO_SLEEP_FOREVER);
    } else {
        Serial.println("--- Battery health OK: " + String(battery.getBatteryVoltage()) + "V ---");
    }
}

void prepareForHibernation() {
    if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT0) {
        // was woken up by user action
        // maybe user wants to do something
        // let's not go to sleep immediately
        Serial.println("--- Going to sleep in 3 min ---");
        hibernationTimer.setTimeout(180000L, hibernate);
    } else {
        Serial.println("--- Going to sleep in 10s ---");
        hibernationTimer.setTimeout(10000L, hibernate);
    }
}


// ------------------------ Methods ---------------------- //

void waitForNextCycle() {
    uint32_t now;
    do { now = millis(); } while (now - cycleTimer.laptime < WAIT_PERIOD);
    cycleTimer.laptime = now;
    cycleTimer.ticks++;
}

void startMeasures() {
    Serial.println("--- Measuring start ---");
    battery.startMeasure();
    moisture.startMeasure();
    waterLevel.startMesure();
}

bool isMeasuringCompleted() {
    return battery.getBatteryVoltage() != -1
        && moisture.getMoisturePercentage() != -1
        && waterLevel.getWaterPercentage() != -1;
}

void waterPlantIfNeeded() {
    Serial.println("--- Moisture " + String(moisture.getMoisturePercentage()) + "% --- threshold: " + moistureThreshold + "% ---");
    if (waterLevel.getWaterPercentage() > 0 && moisture.getMoisturePercentage() < moistureThreshold) {
        Serial.println("--- Watering thirsty plant --");
        waterPump.startPumping();
        Blynk.virtualWrite(V6, 1); // log pump action
    } else {
        Serial.println("--- No need to water plant --");
    }
}

// ------------------------ Core ---------------------- //

bool needToStartMeasure = true;
bool needToSendDataBlynk = true;
bool needToCheckWatering = true;
bool needToCheckBatteryHealth = true;

void setup() {
    Serial.begin(115200);

    pinMode(PIN_BUILTIN_LED, OUTPUT);
    digitalWrite(PIN_BUILTIN_LED, LOW);

    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
    Blynk.syncVirtual(V4);
}

void loop() {
    Blynk.run();
    hibernationTimer.run();
    pumpTimer.run();

    button.loopRoutine();
    battery.loopRoutine();
    moisture.loopRoutine();
    waterPump.loopRoutine();
    waterLevel.loopRoutine();

    if (needToStartMeasure) {
        needToStartMeasure = false;
        startMeasures();
    }

    if (isMeasuringCompleted()) {
        if (needToCheckBatteryHealth) {
            needToCheckBatteryHealth = false;
            sleepForeverIfNeeded();
        }

        if (needToCheckWatering) {
            needToCheckWatering = false;
            waterPlantIfNeeded();
        }
        

        if (needToSendDataBlynk) {
            needToSendDataBlynk = false;
            sendDataToBlynk();
            prepareForHibernation();
        }   
    }

    // if (button.held(50)) { // emergency stop
    //     Serial.println("Held > 50");
    //     waterPump.stopPumping();
    // }
    
    waitForNextCycle();
}