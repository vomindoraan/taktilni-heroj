#include "common.h"

#ifndef DEBUG
#   define DEBUG 1  // 0–2
#endif

#define FIREFEEL_5WAY    'F'  // Firefeel ST01 Strat 5-way WH lever switch
#define OAKGRIGSBY_6WAY  'O'  // Oak-Grigsby 6-way pickup selector lever switch
#ifndef SELECTOR_TYPE
#   define SELECTOR_TYPE FIREFEEL_5WAY
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

using time_t = unsigned long;

class Switch {
public:
    byte const pin;
    bool const activeState;

    Switch(byte pin, bool activeState = LOW) :
        pin{pin},
        activeState{activeState}
    {}

    bool active() const {
        return digitalRead(pin) == activeState;
    }
};

class Button : public Switch {
private:
    bool   state;
    bool   lastState;
    bool   wasPressed;
    time_t debounceDelay;
    time_t lastDebounceTime;

public:
    Button(byte pin, bool activeState = LOW, time_t debounceDelay = 50) :
        Switch{pin, activeState},
        state{!activeState},
        lastState{!activeState},
        wasPressed{false},
        debounceDelay{debounceDelay},
        lastDebounceTime{0}
    {}

    bool pressed() {
        wasPressed = state == activeState;
        bool reading = digitalRead(pin);
        time_t currentTime = millis();
        if (reading != lastState) {
            lastDebounceTime = currentTime;
        }
        if (currentTime - lastDebounceTime > debounceDelay && reading != state) {
            state = reading;
        }
        lastState = reading;
        return state == activeState;
    }

    bool toggled() {
        return pressed() ^ wasPressed;
    }

    bool toggledOn() {
        return pressed() && !wasPressed;
    }

    bool toggledOff() {
        return !pressed() && wasPressed;
    }
};

Switch switchForward = {SWITCH_FORWARD_PIN, LOW};
Switch switchReverse = {SWITCH_REVERSE_PIN, LOW};
Button buttonForward = {BUTTON_FORWARD_PIN, LOW};
Button buttonReverse = {BUTTON_REVERSE_PIN, LOW};

Button selector[] = {
    {SELECTOR_PIN1, LOW},
    {SELECTOR_PIN2, LOW},
    {SELECTOR_PIN3, LOW},
#if SELECTOR_TYPE == OAKGRIGSBY_6WAY
    {SELECTOR_PIN4, LOW},
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
    for (auto& s : selector) {
        pinMode(s.pin, INPUT_PULLUP);
    }

#if SELECTOR_TYPE == FIREFEEL_5WAY
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
#else
#   error "Invalid selector type"
#endif

    Serial.begin(SERIAL_BAUD_RATE);   // USB serial for logging
    Serial1.begin(SERIAL_BAUD_RATE);  // HW serial to color_to_sound
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
    byte state = 0;
    for (int i = 0; i < ARRAY_LEN(selector); i++) {
        if (selector[i].toggled()) {
            state |= selector[i].active() << i;
        }
    }
    if (state && modeMap[state]) {
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
