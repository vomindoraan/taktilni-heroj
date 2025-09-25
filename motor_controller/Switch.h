#pragma once

using time_t = unsigned long;

struct Switch {
    byte const pin;
    bool const activeState;

    Switch(byte pin, bool activeState = LOW) :
        pin{pin},
        activeState{activeState}
    {}

    virtual ~Switch() {}

    virtual bool active() const {
        return digitalRead(pin) == activeState;
    }
};

class DebouncedButton : public Switch {
private:
    bool   state;
    bool   lastReading;
    bool   wasPressed;
    time_t debounceDelay;
    time_t lastReadingTime;
    time_t lastDebounceTime;

public:
    DebouncedButton(byte pin, bool activeState = LOW, time_t debounceDelay = 50) :
        Switch{pin, activeState},
        state{!activeState},
        lastReading{!activeState},
        wasPressed{false},
        debounceDelay{debounceDelay},
        lastReadingTime{0},
        lastDebounceTime{0}
    {}

    bool active() const override {
        return (millis() - lastReadingTime < debounceDelay)
            ? state == activeState
            : Switch::active();
    }

    bool pressed() {
        wasPressed = state == activeState;
        bool reading = digitalRead(pin);
        lastReadingTime = millis();
        if (reading != lastReading) {
            lastDebounceTime = lastReadingTime;
        }
        if (lastReadingTime - lastDebounceTime > debounceDelay && reading != state) {
            state = reading;
        }
        lastReading = reading;
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

using Button = DebouncedButton;
