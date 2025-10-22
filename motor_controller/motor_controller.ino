#include "common.h"
#include "Switch.h"

#ifndef USE_DISPLAY
#   define USE_DISPLAY false
#endif
#if USE_DISPLAY
#   include <TM1637Display.h>
#endif

#define GENERIC_TL_5WAY  '5'  // Generic Telecaster 5-way lever switch (KP)
#define OAKGRIGSBY_6WAY  '6'  // Oak-Grigsby MX3070 6-way lever switch
#define FIREFEEL_ST_5WAY 'F'  // Firefeel ST01 Stratocaster 5-way lever switch
#ifndef SELECTOR_TYPE
#   define SELECTOR_TYPE OAKGRIGSBY_6WAY
#endif

#ifndef DEBUG
#   define DEBUG 1  // 0–2
#endif

#define MOTOR_POWER_PIN     21
#define MOTOR_DIRECTION_PIN 20
#define POT_SPEED_PIN       19
#define SWITCH_FORWARD_PIN  9
#define SWITCH_REVERSE_PIN  8
#define BUTTON_FORWARD_PIN  7
#define BUTTON_REVERSE_PIN  6
#define SELECTOR_PIN1       2
#define SELECTOR_PIN2       3
#define SELECTOR_PIN3       4
#if SELECTOR_TYPE == OAKGRIGSBY_6WAY
#   define SELECTOR_PIN4    5
#endif
#if USE_DISPLAY
#   define DISPLAY_CLK_PIN  15
#   define DISPLAY_DIO_PIN  14
#endif

Switch switchForward = {SWITCH_FORWARD_PIN};
Switch switchReverse = {SWITCH_REVERSE_PIN};
Button buttonForward = {BUTTON_FORWARD_PIN};
Button buttonReverse = {BUTTON_REVERSE_PIN};

Button selector[] = {
    {SELECTOR_PIN1},
    {SELECTOR_PIN2},
    {SELECTOR_PIN3},
#if SELECTOR_TYPE == OAKGRIGSBY_6WAY
    {SELECTOR_PIN4},
#endif
};

mode_t modeMap[1<<ARRLEN(selector)];  // Selector state → mode

#if USE_DISPLAY
TM1637Display display{DISPLAY_CLK_PIN, DISPLAY_DIO_PIN};
#endif

void setup() {
    pinMode(MOTOR_POWER_PIN,     OUTPUT);
    pinMode(MOTOR_DIRECTION_PIN, OUTPUT);
    pinMode(SWITCH_FORWARD_PIN,  INPUT_PULLUP);
    pinMode(SWITCH_REVERSE_PIN,  INPUT_PULLUP);
    pinMode(BUTTON_FORWARD_PIN,  INPUT);
    pinMode(BUTTON_REVERSE_PIN,  INPUT);
    for (auto&& s : selector) {
        pinMode(s.pin, INPUT_PULLUP);
    }

#if SELECTOR_TYPE == GENERIC_TL_5WAY
    // Pins:  876 + 5(GND)
    modeMap[0b100] = 1;
    modeMap[0b110] = 2;
    modeMap[0b010] = 3;
    modeMap[0b011] = 4;
    modeMap[0b001] = 6;
#elif SELECTOR_TYPE == OAKGRIGSBY_6WAY
    // Pins: A4321 + A0(GND)
    modeMap[0b1000] = 1;
    modeMap[0b1100] = 2;
    modeMap[0b0100] = 3;
    modeMap[0b0110] = 4;
    modeMap[0b0010] = 5;
    modeMap[0b0011] = 6;
#elif SELECTOR_TYPE == FIREFEEL_ST_5WAY
#   error "Not implemented"
#else
#   error "Invalid selector type"
#endif

    stop();  // Prevent movement at startup
#if USE_DISPLAY
    display.clear();
#endif

    Serial.begin(SERIAL_BAUD_RATE);   // USB serial for logging
    Serial1.begin(SERIAL_BAUD_RATE);  // HW serial to color_to_sound
    delay(SERIAL_BEGIN_DELAY);
}

void loop() {
    checkSelector();

    checkSync();

    if (switchForward.active()) {
        forward();
    } else if (switchReverse.active()) {
        reverse();
    } else if (buttonForward.pressed()) {
        forward();
    } else if (buttonReverse.pressed()) {
        reverse();
    } else {
        stop();
    }
}

void checkSync() {
    static time_ms lastReadingTime, lastSyncTime, syncPeriod;
    time_ms currTime = millis();

    // Update period/BPM based on pot ADC (lower value = higher speed)
    if (currTime - lastReadingTime >= SYNC_PERIOD_LOW) {
        int reading = analogRead(POT_SPEED_PIN);
        lastReadingTime = currTime;
        syncPeriod = map(
            constrain(reading, 0, 1023),
            0, 1023,
            SYNC_PERIOD_LOW, SYNC_PERIOD_HIGH
        );
#if USE_DISPLAY
        display.showNumberDec(BPM(syncPeriod));
#endif
    }

    if (currTime - lastSyncTime >= syncPeriod) {
        sync();
        lastSyncTime = currTime;
    }
}

void checkSelector() {
    bool changed = false;
    byte state = 0;
    for (size_t i = 0; i < ARRLEN(selector); i++) {
        changed |= selector[i].toggled();
        state |= selector[i].active() << i;
    }
    if (changed && modeMap[state]) {
        changeMode(modeMap[state]);
    }
}

void sync() {
    Serial1.write(CMD_SYNC);
#if DEBUG
    Serial.println("Sync");
#endif
}

void changeMode(mode_t mode) {
    Serial1.write(CMD_CHANGE_MODE);
    Serial1.print(mode);
#if DEBUG
    Serial.print("Mode "); Serial.println(mode);
#endif
}

void forward() {
    digitalWrite(MOTOR_POWER_PIN,     LOW);
    digitalWrite(MOTOR_DIRECTION_PIN, HIGH);
#if DEBUG >= 2
    Serial.println("Forward");
#endif
}

void reverse() {
    digitalWrite(MOTOR_POWER_PIN,     LOW);
    digitalWrite(MOTOR_DIRECTION_PIN, LOW);
#if DEBUG >= 2
    Serial.println("Reverse");
#endif
}

void stop() {
    digitalWrite(MOTOR_POWER_PIN, HIGH);
#if DEBUG >= 2
    Serial.println("Stop");
#endif
}
