#pragma once

using time_t = unsigned long;

struct Switch {
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

class DebouncedButton : public Switch {
private:
    bool   state;
    bool   lastState;
    bool   wasPressed;
    time_t debounceDelay;
    time_t lastDebounceTime;

public:
    DebouncedButton(byte pin, bool activeState = LOW, time_t debounceDelay = 50) :
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

using Button = DebouncedButton;
