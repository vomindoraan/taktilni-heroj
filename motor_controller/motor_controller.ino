#define SWITCH_FORWARD_PIN  9
#define SWITCH_REVERSE_PIN  8
#define BUTTON_FORWARD_PIN  16
#define BUTTON_REVERSE_PIN  15
#define MOTOR_POWER_PIN     21
#define MOTOR_DIRECTION_PIN 20

void setup() {
    pinMode(SWITCH_FORWARD_PIN,  INPUT_PULLUP);
    pinMode(SWITCH_REVERSE_PIN,  INPUT_PULLUP);
    pinMode(BUTTON_FORWARD_PIN,  INPUT);
    pinMode(BUTTON_REVERSE_PIN,  INPUT);
    pinMode(MOTOR_POWER_PIN,     OUTPUT);
    pinMode(MOTOR_DIRECTION_PIN, OUTPUT);

    Serial.begin(115200);
}

void loop() {
    if (digitalRead(SWITCH_FORWARD_PIN) == LOW) {
        play_forward();
    } else if (digitalRead(SWITCH_REVERSE_PIN) == LOW) {
        play_reverse();
    } else {
        stop();
        // if (digitalRead(BUTTON_FORWARD_PIN) == LOW) {
        //     step_forward();
        // } else if (digitalRead(BUTTON_REVERSE_PIN) == LOW) {
        //     step_reverse();
        // }
    }
}

void play_forward() {
    digitalWrite(MOTOR_POWER_PIN,     LOW);
    digitalWrite(MOTOR_DIRECTION_PIN, HIGH);
    Serial.println("Playing forward");
}

void play_reverse() {
    digitalWrite(MOTOR_POWER_PIN,     LOW);
    digitalWrite(MOTOR_DIRECTION_PIN, LOW);
    Serial.println("Playing reverse");
}

void step_forward() {

}

void step_reverse() {

}

void stop() {
    digitalWrite(MOTOR_POWER_PIN, HIGH);
    Serial.println("Stopped");
}