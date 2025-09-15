#include "common.h"

#ifndef DEBUG
#   define DEBUG 1  // 0–2
#endif
#if DEBUG
#   warning "Serial debug may interfere with commands sent to other devices"
#endif

#define MOTOR_POWER_PIN     21
#define MOTOR_DIRECTION_PIN 20
#define SWITCH_FORWARD_PIN  9
#define SWITCH_REVERSE_PIN  8
#define BUTTON_FORWARD_PIN  16
#define BUTTON_REVERSE_PIN  15
#define SELECTOR_MODE_PIN1  2
#define SELECTOR_MODE_PIN2  3
#define SELECTOR_MODE_PIN3  4

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

    bool pressedOn() {
        return pressed() && !wasPressed;
    }

    bool pressedOff() {
        return !pressed() && wasPressed;
    }
};

Switch switchForward = {SWITCH_FORWARD_PIN, LOW};
Switch switchReverse = {SWITCH_REVERSE_PIN, LOW};
Button buttonForward = {BUTTON_FORWARD_PIN, LOW};
Button buttonReverse = {BUTTON_REVERSE_PIN, LOW};

Button selector[] = {
    {SELECTOR_MODE_PIN1, LOW},
    {SELECTOR_MODE_PIN2, LOW},
    {SELECTOR_MODE_PIN3, LOW},
};

int modeMap[1<<ARRAY_LEN(selector)] = {0};  // selector state → mode

void setup() {
    modeMap[0b100] = 1;
    modeMap[0b110] = 2;
    modeMap[0b010] = 3;
    modeMap[0b011] = 4;
    //modeMap[0b001] = 5;  // TODO: NC until we get a 6-way switch
    modeMap[0b001] = 6;

    pinMode(MOTOR_POWER_PIN,     OUTPUT);
    pinMode(MOTOR_DIRECTION_PIN, OUTPUT);
    pinMode(SWITCH_FORWARD_PIN,  INPUT_PULLUP);
    pinMode(SWITCH_REVERSE_PIN,  INPUT_PULLUP);
    pinMode(BUTTON_FORWARD_PIN,  INPUT);
    pinMode(BUTTON_REVERSE_PIN,  INPUT);
    for (auto& s : selector) {
        pinMode(s.pin, INPUT_PULLUP);
    }

    Serial.begin(SERIAL_BAUD_RATE);
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
    bool changed = false;
    for (int i = 0; i < ARRAY_LEN(selector); i++) {
        if (selector[i].active()) {
            state |= 1 << i;
            changed |= selector[i].pressedOn();  // TODO: pressedOff?
        }
    }
    if (changed) {
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
    Serial.write(CHANGE_MODE_CMD);
    Serial.print(mode);
#if DEBUG
    Serial.println(" - Change mode");
#endif
}
