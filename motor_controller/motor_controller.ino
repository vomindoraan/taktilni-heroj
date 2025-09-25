#include "common.h"
#include "Switch.h"

#ifndef DEBUG
#   define DEBUG 1  // 0–2
#endif

#define GENERIC_TL_5WAY  '5'  // Generic Telecaster 5-way lever switch (KP)
#define OAKGRIGSBY_6WAY  '6'  // Oak-Grigsby MX3070 6-way lever switch
#define FIREFEEL_ST_5WAY 'F'  // Firefeel ST01 Stratocaster 5-way lever switch
#ifndef SELECTOR_TYPE
#   define SELECTOR_TYPE GENERIC_TL_5WAY
#endif

#define MOTOR_POWER_PIN     21
#define MOTOR_DIRECTION_PIN 20
#define SWITCH_FORWARD_PIN  9
#define SWITCH_REVERSE_PIN  8
#define BUTTON_FORWARD_PIN  16
#define BUTTON_REVERSE_PIN  15
#define SELECTOR_PIN1       2
#define SELECTOR_PIN2       3
#define SELECTOR_PIN3       4
#if SELECTOR_TYPE == OAKGRIGSBY_6WAY
#   define SELECTOR_PIN4    5
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

int modeMap[1<<ARRAY_LEN(selector)] = {0};  // selector state → mode

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
    modeMap[0b0011] = 1;
    modeMap[0b0010] = 2;
    modeMap[0b0110] = 3;
    modeMap[0b0100] = 4;
    modeMap[0b1100] = 5;
    modeMap[0b1000] = 6;
#elif SELECTOR_TYPE == FIREFEEL_ST_5WAY
#   error "Not implemented"
#else
#   error "Invalid selector type"
#endif

    stop();  // Prevent track from moving at startup

    Serial.begin(SERIAL_BAUD_RATE);   // USB serial for logging
    Serial1.begin(SERIAL_BAUD_RATE);  // HW serial to color_to_sound
    delay(SERIAL_BEGIN_DELAY);
}

void loop() {
    checkSelector();

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

void checkSelector() {
    bool changed = false;
    byte state = 0;
    for (int i = 0; i < ARRAY_LEN(selector); i++) {
        changed |= selector[i].toggled();
        state |= selector[i].active() << i;
    }
    if (changed && modeMap[state]) {
        changeMode(modeMap[state]);
    }
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

void changeMode(int mode) {
    Serial1.write(CHANGE_MODE_CMD);
    Serial1.print(mode);
#if DEBUG
    Serial.print("Mode "); Serial.println(mode);
#endif
}
