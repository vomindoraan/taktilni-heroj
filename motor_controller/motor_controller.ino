// Pro Micro
#include "common.h"
#include "Switch.h"

#ifndef DEBUG
#   define DEBUG 2  // 0–3
#endif

#ifndef DISPLAY_BPM
#   define DISPLAY_BPM true
#endif
#if DISPLAY_BPM
#   include <TM1637Display.h>
#endif

#ifndef SEND_MIDI
#   define SEND_MIDI true
#endif
#if SEND_MIDI
#   include <MIDI.h>
#   include <MIDIUSB.h>
#endif

#define GENERIC_TL_5WAY  '5'  // Generic Telecaster 5-way lever switch (KP)
#define OAKGRIGSBY_6WAY  '6'  // Oak-Grigsby MX3070 6-way lever switch
#define FIREFEEL_ST_5WAY 'F'  // Firefeel ST01 Stratocaster 5-way lever switch
#ifndef SELECTOR_TYPE
#   define SELECTOR_TYPE OAKGRIGSBY_6WAY
#endif

#define MOTOR_POWER_PIN     21
#define MOTOR_DIRECTION_PIN 20
#define POT_SPEED_PIN       19
#define SWITCH_FORWARD_PIN  9
#define SWITCH_REVERSE_PIN  8
#define BUTTON_FORWARD_PIN  7
#define BUTTON_REVERSE_PIN  6
#define SELECTOR_PIN1       2
#define SELECTOR_PIN2       3
#define SELECTOR_PIN3       4
#if SELECTOR_TYPE == OAKGRIGSBY_6WAY
#   define SELECTOR_PIN4    5
#endif
#if DISPLAY_BPM
#   define DISPLAY_CLK_PIN  15
#   define DISPLAY_DIO_PIN  14
#endif

#if SEND_MIDI
#   define MIDI_PPQN 24
#   define MIDI_PPEN (MIDI_PPQN / 2)
USING_NAMESPACE_MIDI
#endif

Switch switchForward = {SWITCH_FORWARD_PIN};
Switch switchReverse = {SWITCH_REVERSE_PIN};
Button buttonForward = {BUTTON_FORWARD_PIN};
Button buttonReverse = {BUTTON_REVERSE_PIN};

Button selector[] = {
    {SELECTOR_PIN1},
    {SELECTOR_PIN2},
    {SELECTOR_PIN3},
#if SELECTOR_TYPE == OAKGRIGSBY_6WAY
    {SELECTOR_PIN4},
#endif
};

mode_t modeMap[1<<ARRLEN(selector)];  // Selector state → mode

bool playing;  // Is the conveyor currently moving and playing?

#if DISPLAY_BPM
TM1637Display display{DISPLAY_CLK_PIN, DISPLAY_DIO_PIN};
#endif

void setup() {
    pinMode(MOTOR_POWER_PIN,     OUTPUT);
    pinMode(MOTOR_DIRECTION_PIN, OUTPUT);
    pinMode(SWITCH_FORWARD_PIN,  INPUT_PULLUP);
    pinMode(SWITCH_REVERSE_PIN,  INPUT_PULLUP);
    pinMode(BUTTON_FORWARD_PIN,  INPUT_PULLUP);
    pinMode(BUTTON_REVERSE_PIN,  INPUT_PULLUP);
    for (auto&& s : selector) {
        pinMode(s.pin, INPUT_PULLUP);
    }

#if SELECTOR_TYPE == GENERIC_TL_5WAY
    // Pins:  876 + 5(GND)
    modeMap[0b100] = 1;
    modeMap[0b110] = 2;
    modeMap[0b010] = 3;
    modeMap[0b011] = 4;
    modeMap[0b001] = 6;
#elif SELECTOR_TYPE == OAKGRIGSBY_6WAY
    // Pins: A4321 + A0(GND)
    modeMap[0b1000] = 1;
    modeMap[0b1100] = 2;
    modeMap[0b0100] = 3;
    modeMap[0b0110] = 4;
    modeMap[0b0010] = 5;
    modeMap[0b0011] = 6;
#elif SELECTOR_TYPE == FIREFEEL_ST_5WAY
#   error "Not implemented"
#else
#   error "Invalid selector type"
#endif

    stop();  // Prevent movement at startup
#if DISPLAY_BPM
    display.setBrightness(7);
    display.clear();
#endif

    Serial.begin(SERIAL_BAUD_RATE);   // USB serial for logging and MIDI
    Serial1.begin(SERIAL_BAUD_RATE);  // HW serial to color_to_sound
    delay(SERIAL_BEGIN_DELAY);

#if SEND_MIDI
    sendMIDI(MidiType::Start);  // Reset song pos, start playback
    sendMIDI(MidiType::Stop);   // Stop playback (controls resume)
#endif
}

void loop() {
    checkSelector();  // Writes to color_to_sound via Serial1
    checkSync();      // Writes to color_to_sound via Serial1, MIDI via Serial
    checkControls();  // Writes to MIDI via Serial
}

time_ms syncMap(int input) {
    // Known data points (input, output)
    const double data[][2] = {
        {0, 234.375},
        {155, 254.237288},
        {290, 281.690141},
        {407, 303.030303},
        {556, 338.600451},
        {656, 375.0},
        {825, 465.116279},
        {958, 550.458716},
        {1023, 612.244898}
    };
    const int numPoints = 9;

    // Clamp input to valid range
    if (input <= 0) return 234.375;
    if (input >= 1023) return 612.244898;

    // Find the two points to interpolate between
    for (int i = 0; i < numPoints - 1; i++) {
        if (input >= data[i][0] && input <= data[i+1][0]) {
            double x0 = data[i][0];
            double y0 = data[i][1];
            double x1 = data[i+1][0];
            double y1 = data[i+1][1];

            // Linear interpolation
            double t = (input - x0) / (x1 - x0);
            return y0 + t * (y1 - y0);
        }
    }

    return 234.375; // Fallback
}

void checkSync() {
    static time_ms lastSyncTime, syncPeriod;
#if SEND_MIDI
    static time_ms lastMidiTime;
    static bool doMidi;
#endif
    time_ms currTime = millis();

    // Update period/BPM based on pot ADC (lower value = higher speed)
    int reading = analogRead(POT_SPEED_PIN);
    syncPeriod = syncMap(reading);

    // Send Sync and/or MIDI Clock based on period/BPM
    if (currTime - lastSyncTime >= syncPeriod) {
        sync();
        lastSyncTime = currTime;
#if DISPLAY_BPM
        static bool drawTop;  // Segments:  gfedcba
        static byte const boxTop[] = {0b01100011};
        static byte const boxBtm[] = {0b01011100};
        display.setSegments(drawTop ? boxTop : boxBtm, 1, 0);
        drawTop = !drawTop;
#endif
#if SEND_MIDI
        doMidi = playing;  // Ensure Clock starts and stops on Sync
    }

    if (doMidi && currTime - lastMidiTime >= syncPeriod / MIDI_PPEN) {
        sendMIDI(MidiType::Clock);
        lastMidiTime = currTime;
#endif
    }

#if DISPLAY_BPM
    static time_ms lastDisplayTime;
    if (currTime - lastDisplayTime >= SYNC_PERIOD_HIGH) {
        int bpm = min(BPM(syncPeriod), 999);
        display.showNumberDec(bpm, false, 3, 1);
        lastDisplayTime = currTime;
    }
#endif
}

void checkSelector() {
    bool changed = false;
    byte state = 0;
    for (size_t i = 0; i < ARRLEN(selector); i++) {
        changed |= selector[i].toggled();
        state |= selector[i].active() << i;
    }
    if (changed && modeMap[state]) {
        changeMode(modeMap[state]);
    }
}

void checkControls() {
    bool p = true;
    if (switchForward.active()) {  // Order matters
        forward();
    } else if (switchReverse.active()) {
        reverse();
    } else if (buttonForward.pressed()) {
        forward();
    } else if (buttonReverse.pressed()) {
        reverse();
    } else {
        stop();
        p = false;
    }

#if SEND_MIDI
    if (p != playing) {
        sendMIDI(p ? MidiType::Continue : MidiType::Stop);
    }
#endif
    playing = p;
}

void sync() {
    Serial1.write(CMD_SYNC);
#if DEBUG
    Serial.println("Sync");
#endif
}

void changeMode(mode_t mode) {
    Serial1.write(CMD_CHANGE_MODE);
    Serial1.print(mode);
#if DEBUG
    Serial.print("Mode "); Serial.println(mode);
#endif
}

#if SEND_MIDI
void sendMIDI(MidiType type) {
    MidiUSB.sendMIDI({0x0F, type, 0, 0});
    MidiUSB.flush();
#   if DEBUG >= 2
    Serial.print("MIDI "); Serial.println(type, HEX);
#   endif
}
#endif

void forward() {
    digitalWrite(MOTOR_POWER_PIN,     LOW);
    digitalWrite(MOTOR_DIRECTION_PIN, HIGH);
#if DEBUG >= 3
    Serial.println("Forward");
#endif
}

void reverse() {
    digitalWrite(MOTOR_POWER_PIN,     LOW);
    digitalWrite(MOTOR_DIRECTION_PIN, LOW);
#if DEBUG >= 3
    Serial.println("Reverse");
#endif
}

void stop() {
    digitalWrite(MOTOR_POWER_PIN, HIGH);
#if DEBUG >= 3
    Serial.println("Stop");
#endif
}
