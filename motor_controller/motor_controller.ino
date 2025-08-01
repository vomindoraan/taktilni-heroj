#ifndef DEBUG
#   define DEBUG 1
#endif

#define SWITCH_FORWARD_PIN  9
#define SWITCH_REVERSE_PIN  8
#define BUTTON_SEEK_PIN     16
#define BUTTON_MODE_PIN     15
#define MOTOR_POWER_PIN     21
#define MOTOR_DIRECTION_PIN 20

class Switch {
protected:
    int  pin;
    bool active_state;

public:
    Switch(int pin, bool active_state = LOW) :
        pin{pin},
        active_state{active_state}
    {}

    bool active() {
        return digitalRead(pin) == active_state;
    }
};

class Button : public Switch {
private:
    bool          state;
    bool          last_state;
    unsigned long debounce_delay;
    unsigned long last_debounce_time;

public:
    Button(int pin, bool active_state = LOW, unsigned long debounce_delay = 50) :
        Switch{pin, active_state},
        state{!active_state},
        last_state{!active_state},
        debounce_delay{debounce_delay},
        last_debounce_time{0}
    {}

    bool pressed() {
        bool reading = digitalRead(pin);
        auto current_time = millis();
        if (reading != last_state) {
            last_debounce_time = current_time;
        }
        bool pressed = false;
        if (current_time - last_debounce_time > debounce_delay && reading != state) {
            state = reading;
            if (state == active_state) {
                pressed = true;
            }
        }
        last_state = reading;
        return pressed;
    }

    bool pressedOn() {
        return state != active_state && pressed();
    }
};

Switch switch_forward = {SWITCH_FORWARD_PIN, LOW};
Switch switch_reverse = {SWITCH_REVERSE_PIN, LOW};
Button button_seek    = {BUTTON_SEEK_PIN,    LOW};
Button button_mode    = {BUTTON_MODE_PIN,    LOW};

void setup() {
    pinMode(SWITCH_FORWARD_PIN,  INPUT_PULLUP);
    pinMode(SWITCH_REVERSE_PIN,  INPUT_PULLUP);
    pinMode(BUTTON_SEEK_PIN,     INPUT);
    pinMode(BUTTON_MODE_PIN,     INPUT);
    pinMode(MOTOR_POWER_PIN,     OUTPUT);
    pinMode(MOTOR_DIRECTION_PIN, OUTPUT);

    Serial.begin(115200);
}

void loop() {
    if (switch_forward.active()) {
        forward();
    } else if (switch_reverse.active()) {
        reverse();
    } else if (button_seek.pressed()) {
        forward();
    } else {
        stop();
    }

    if (button_mode.pressedOn()) {
        change_mode();
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

void change_mode() {
    // TODO
#if DEBUG
    Serial.println("Change mode");
#endif
}
