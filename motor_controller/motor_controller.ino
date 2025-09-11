#include "common.h"

#ifndef DEBUG
#   define DEBUG 0
#endif
#if DEBUG
#   warning "Serial debug may interfere with commands sent to other devices"
#endif

#define SWITCH_FORWARD_PIN  9
#define SWITCH_REVERSE_PIN  8
#define BUTTON_SEEK_PIN     16
#define BUTTON_MODE_PIN     15
#define MOTOR_POWER_PIN     21
#define MOTOR_DIRECTION_PIN 20

class Switch {
public:
    int const  pin;
    bool const activeState;

    Switch(int pin, bool activeState = LOW) :
        pin{pin},
        activeState{activeState}
    {}

    bool active() const {
        return digitalRead(pin) == activeState;
    }
};

class Button : public Switch {
private:
    bool          state;
    bool          lastState;
    bool          wasPressed;
    unsigned long debounceDelay;
    unsigned long lastDebounceTime;

public:
    Button(int pin, bool activeState = LOW, unsigned long debounceDelay = 50) :
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
        auto currentTime = millis();
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
Button buttonSeek    = {BUTTON_SEEK_PIN,    LOW};
Button buttonMode    = {BUTTON_MODE_PIN,    LOW};

void setup() {
    pinMode(SWITCH_FORWARD_PIN,  INPUT_PULLUP);
    pinMode(SWITCH_REVERSE_PIN,  INPUT_PULLUP);
    pinMode(BUTTON_SEEK_PIN,     INPUT);
    pinMode(BUTTON_MODE_PIN,     INPUT);
    pinMode(MOTOR_POWER_PIN,     OUTPUT);
    pinMode(MOTOR_DIRECTION_PIN, OUTPUT);

    Serial.begin(SERIAL_BAUD_RATE);
}

void loop() {
    if (switchForward.active()) {
        forward();
    } else if (switchReverse.active()) {
        reverse();
    } else if (buttonSeek.pressed()) {
        forward();
    } else {
        stop();
    }

    if (buttonMode.pressedOn()) {
        changeMode();
    }
}

void forward() {
    digitalWrite(MOTOR_POWER_PIN,     LOW);
    digitalWrite(MOTOR_DIRECTION_PIN, HIGH);
#if DEBUG
    Serial.println("Forward");
#endif
}

void reverse() {
    digitalWrite(MOTOR_POWER_PIN,     LOW);
    digitalWrite(MOTOR_DIRECTION_PIN, LOW);
#if DEBUG
    Serial.println("Reverse");
#endif
}

void stop() {
    digitalWrite(MOTOR_POWER_PIN, HIGH);
#if DEBUG
    Serial.println("Stop");
#endif
}

void changeMode() {
    Serial.write(CHANGE_MODE_CMD);
#if DEBUG
    Serial.println("Change mode");
#endif
}
