#include "common.h"

#ifndef DEBUG
#   define DEBUG 0
#endif
#if DEBUG
#   warning "Serial debug may interfere with commands sent to other devices"
#endif

#define SWITCH_FORWARD_PIN  9
#define SWITCH_REVERSE_PIN  8
#define BUTTON_FORWARD_PIN  16
#define BUTTON_REVERSE_PIN  15
#define SELECTOR_MODE1_PIN  2
#define SELECTOR_MODE2_PIN  3
#define SELECTOR_MODE3_PIN  4
#define SELECTOR_MODE4_PIN  5
#define SELECTOR_MODE5_PIN  6  // TODO: NC until we get a 6-way switch
#define SELECTOR_MODE6_PIN  7
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
    unsigned long debounceDelay;
    unsigned long lastDebounceTime;

public:
    Button(int pin, bool activeState = LOW, unsigned long debounceDelay = 50) :
        Switch{pin, activeState},
        state{!activeState},
        lastState{!activeState},
        debounceDelay{debounceDelay},
        lastDebounceTime{0}
    {}

    bool pressed() {
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
        return state != activeState && pressed();
    }

    bool pressedOff() {
        return state == activeState && !pressed();
    }
};

Switch switchForward = {SWITCH_FORWARD_PIN, LOW};
Switch switchReverse = {SWITCH_REVERSE_PIN, LOW};
Button buttonForward = {BUTTON_FORWARD_PIN, LOW};
Button buttonReverse = {BUTTON_REVERSE_PIN, LOW};

Button selectorMode[] = {
    {SELECTOR_MODE1_PIN, LOW},
    {SELECTOR_MODE2_PIN, LOW},
    {SELECTOR_MODE3_PIN, LOW},
    {SELECTOR_MODE4_PIN, LOW},
    {SELECTOR_MODE5_PIN, LOW},
    {SELECTOR_MODE6_PIN, LOW},
};

void setup() {
    pinMode(SWITCH_FORWARD_PIN,  INPUT_PULLUP);
    pinMode(SWITCH_REVERSE_PIN,  INPUT_PULLUP);
    pinMode(BUTTON_FORWARD_PIN,  INPUT);
    pinMode(BUTTON_REVERSE_PIN,  INPUT);
    pinMode(MOTOR_POWER_PIN,     OUTPUT);
    pinMode(MOTOR_DIRECTION_PIN, OUTPUT);
    for (auto& s : selectorMode) {
        pinMode(s.pin, INPUT_PULLUP);
    }

    Serial.begin(SERIAL_BAUD_RATE);
}

void loop() {
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

    for (int i = 0; i < sizeof selectorMode / sizeof *selectorMode; i++) {
        if (selectorMode[i].pressedOn()) {
            changeMode(i + 1);
            break;
        }
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

void changeMode(int mode) {
    Serial.write(CHANGE_MODE_CMD);
    Serial.print(mode);
#if DEBUG
    Serial.println(" - Change mode");
#endif
}
