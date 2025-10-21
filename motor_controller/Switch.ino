#include "Switch.h"

bool Switch::active() const {
    return digitalRead(pin) == activeState;
}

bool DebouncedButton::active() const {
    return (millis() - lastReadingTime < debounceDelay)
        ? state == activeState
        : Switch::active();
}

bool DebouncedButton::pressed() {
    wasPressed = state == activeState;

    bool reading = digitalRead(pin);
    lastReadingTime = millis();
    if (reading != lastReading) {
        lastDebounceTime = lastReadingTime;
    }
    if (lastReadingTime - lastDebounceTime >= debounceDelay && reading != state) {
        state = reading;
    }
    lastReading = reading;

    return state == activeState;
}

bool DebouncedButton::toggled() {
    return pressed() != wasPressed;
}

bool DebouncedButton::toggledOn() {
    return pressed() && !wasPressed;
}

bool DebouncedButton::toggledOff() {
    return !pressed() && wasPressed;
}
