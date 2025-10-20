#pragma once

using time_ms = unsigned long;

struct Switch {
    byte const pin;
    bool const activeState;

    Switch(byte pin, bool activeState = LOW) :
        pin{pin},
        activeState{activeState}
    {}

    virtual ~Switch() {}

    virtual bool active() const;
};

class DebouncedButton : public Switch {
private:
    bool    state;
    bool    lastReading;
    bool    wasPressed;
    time_ms debounceDelay;
    time_ms lastReadingTime;
    time_ms lastDebounceTime;

public:
    DebouncedButton(byte pin, bool activeState = LOW, time_ms debounceDelay = 50);

    bool active() const override;
    bool pressed();
    bool toggled();
    bool toggledOn();
    bool toggledOff();
};

using Button = DebouncedButton;
