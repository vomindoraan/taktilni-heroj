#include <Adafruit_TCS34725.h>
#include <DFMiniMp3.h>
#include <SoftwareSerial.h>

#include "common.h"

#ifndef SENSOR_NO
#   define SENSOR_NO 1  // 1–4
#endif
#ifndef DEBUG
#   define DEBUG     1  // 0–2
#endif

#define COLOR_COUNT           7
#define COLOR_C_THRESHOLD     160
#define COLOR_DIST_THRESHOLD  15
#define COLOR_CDIST_THRESHOLD 100

#define MP3_SERIAL_RX_PIN  9
#define MP3_SERIAL_TX_PIN  8
#define MP3_DEFAULT_VOLUME 30  // 0–30
#define MP3_DEFAULT_FOLDER Folder::TONES

enum Folder : uint8_t {
    TONES = 1,
    DRUMS,
    WORDS,
    AMBIENCE,
    ANIMALS,
    MIXED,  // Mix of 1–4, one folder per sensor
    _TOTAL_FOLDERS = MIXED
};

enum Track : uint8_t {
    C_MAJOR = 1,
    D_MAJOR,
    G_MAJOR,
    D,
    E,
    F_SHARP,
    A,
    _TOTAL_TRACKS = A
};

enum Color : uint16_t {
    NONE  = UINT16_MAX,
    BLACK = 0,
    RED,
    GREEN,
    BLUE,
    YELLOW,
    PURPLE,
    CYAN,
    ORANGE,
    PINK,
    AZURE,
    WHITE,
    _TOTAL_COLORS
};

uint16_t const COLOR_SAMPLES[COLOR_COUNT][5] = {
#if SENSOR_NO == 1
    {Color::RED,    153, 52,  39,  242},
    {Color::GREEN,  77,  118, 67,  264},
    {Color::BLUE,   49,  78,  102, 227},
    {Color::YELLOW, 464, 301, 117, 943},
    {Color::CYAN,   136, 261, 197, 616},
    {Color::ORANGE, 371, 124, 78,  580},
    {Color::PINK,   380, 306, 253, 985},
#elif SENSOR_NO == 2
    {Color::RED,    233, 84,  60,  359},
    {Color::GREEN,  128, 166, 93,  384},
    {Color::BLUE,   83,  104, 119, 296},
    {Color::YELLOW, 607, 397, 173, 1213},
    {Color::CYAN,   182, 272, 194, 650},
    {Color::ORANGE, 500, 175, 110, 766},
    {Color::PINK,   512, 388, 305, 1216},
#elif SENSOR_NO == 3
    {Color::RED,    150, 63,  49,  250},
    {Color::GREEN,  72,  128, 73,  271},
    {Color::BLUE,   41,  74,  91,  200},
    {Color::YELLOW, 343, 250, 112, 718},
    {Color::CYAN,   117, 235, 180, 528},
    {Color::ORANGE, 336, 125, 84,  529},
    {Color::PINK,   360, 315, 262, 938},
#elif SENSOR_NO == 4
    {Color::RED,    222, 73,  47,  351},
    {Color::GREEN,  111, 136, 69,  335},
    {Color::BLUE,   78,  89,  98,  278},
    {Color::YELLOW, 539, 303, 117, 1034},
    {Color::CYAN,   164, 244, 171, 615},
    {Color::ORANGE, 490, 150, 90,  755},
    {Color::PINK,   477, 328, 253, 1123},
#else
#   error "Invalid SENSOR_NO"
#endif
};

Adafruit_TCS34725 tcs{TCS34725_INTEGRATIONTIME_101MS, TCS34725_GAIN_4X};

SoftwareSerial mp3Serial{MP3_SERIAL_RX_PIN, MP3_SERIAL_TX_PIN};

class Mp3Callbacks;
using DfMp3 = DFMiniMp3<SoftwareSerial, Mp3Callbacks>;
DfMp3 mp3{mp3Serial};

Folder mp3Folder = MP3_DEFAULT_FOLDER;
Track  mp3TrackMap[Color::_TOTAL_COLORS];  // Color → track no.

void setup() {
    mp3TrackMap[Color::RED]    = Track::C_MAJOR;
    mp3TrackMap[Color::GREEN]  = Track::G_MAJOR;
    mp3TrackMap[Color::BLUE]   = Track::D_MAJOR;
    mp3TrackMap[Color::YELLOW] = Track::E;
    mp3TrackMap[Color::CYAN]   = Track::F_SHARP;
    mp3TrackMap[Color::ORANGE] = Track::D;
    mp3TrackMap[Color::PINK]   = Track::A;

    Serial.begin(SERIAL_BAUD_RATE);        // USB serial for logging
    Serial1.begin(SERIAL_BAUD_RATE);       // HW serial from motor_controller
    mp3Serial.begin(SW_SERIAL_BAUD_RATE);  // SW serial to/from DFMiniMp3
    delay(SERIAL_BEGIN_DELAY);

    mp3.begin(SW_SERIAL_BAUD_RATE);
#if DEBUG
    mp3.reset();
#endif
    mp3.setVolume(MP3_DEFAULT_VOLUME);

    if (!tcs.begin()) {
        Serial.println("[TCS] No sensor found");
        WAIT_FOREVER();
    }
#if DEBUG
    else {
        Serial.println("[TCS] Sensor " STR(SENSOR_NO) " online");
    }
#endif
}

void loop() {
    static Color lastColor = Color::NONE;

    readChangeMode();  // Reads from motor_controller via Serial1

    uint16_t r, g, b, c;
    readRGBC(r, g, b, c);  // Reads from TCS34725 via I2C

    Color color = identifyColor(r, g, b, c);
    if (color != Color::NONE) {
#if DEBUG
        Serial.print("[TCS] Identified color "); Serial.println(color);
#endif
        if (color != lastColor) {
            playTrackFor(color);  // Writes to DFMiniMp3 via mp3Serial
        }
    }
    lastColor = color;
}

void readRGBC(uint16_t& r, uint16_t& g, uint16_t& b, uint16_t& c) {
    tcs.getRawData(&r, &g, &b, &c);
#if DEBUG >= 2
    Serial.print("[TCS] ");
    Serial.print("R: "); Serial.print(r); Serial.print(", ");
    Serial.print("G: "); Serial.print(g); Serial.print(", ");
    Serial.print("B: "); Serial.print(b); Serial.print(", ");
    Serial.print("C: "); Serial.print(c); Serial.println();
#endif
}

Color identifyColor(uint16_t r, uint16_t g, uint16_t b, uint16_t c) {
    // Ignore background
    if (c < COLOR_C_THRESHOLD) {
        return Color::NONE;
    }

    Color bestMatch = Color::NONE;
    uint16_t bestMatchDist = UINT16_MAX;
    uint16_t bestMatchCDist = UINT16_MAX;

#if DEBUG >= 2
    Serial.print("[TCS] Distances: ");
#endif
    for (int i = 0; i < COLOR_COUNT; i++) {
        auto [color, rSample, gSample, bSample, cSample] = COLOR_SAMPLES[i];
        uint16_t dist = colorDistance(r, g, b, c, rSample, gSample, bSample, cSample);
        uint16_t cDist = abs(int16_t(c) - int16_t(cSample));
#if DEBUG >= 2
        Serial.print(dist); Serial.print("/"); Serial.print(cDist);
        Serial.print((i < COLOR_COUNT-1) ? ", " : "\n");
#endif
        if (
            dist < COLOR_DIST_THRESHOLD && cDist < COLOR_CDIST_THRESHOLD &&
            dist < bestMatchDist        && cDist < bestMatchCDist
        ) {
            bestMatch = Color(color);
            bestMatchDist = dist;
            bestMatchCDist = cDist;
        }
    }

    return bestMatch;
}

uint16_t colorDistance(
    uint16_t r, uint16_t g, uint16_t b, uint16_t c,
    uint16_t rSample, uint16_t gSample, uint16_t bSample, uint16_t cSample
) {
    // Normalize values
    r = (uint32_t(r) << 8) / c;
    g = (uint32_t(g) << 8) / c;
    b = (uint32_t(b) << 8) / c;
    rSample = (uint32_t(rSample) << 8) / cSample;
    gSample = (uint32_t(gSample) << 8) / cSample;
    bSample = (uint32_t(bSample) << 8) / cSample;

    int16_t rDiff = int16_t(r) - int16_t(rSample);
    int16_t gDiff = int16_t(g) - int16_t(gSample);
    int16_t bDiff = int16_t(b) - int16_t(bSample);
    return sqrt(sq(rDiff) + sq(gDiff) + sq(bDiff));
}

void readChangeMode() {
    int mode = 0;
    // Consume consecutive commands, keep latest (format: "M%d")
    while (Serial1.available() && Serial1.read() == CHANGE_MODE_CMD) {
        mode = Serial1.parseInt(SKIP_NONE);
    }

    if (mode > 0 && mode <= Folder::_TOTAL_FOLDERS) {
        mp3Folder = Folder(mode);
#if DEBUG
        Serial.print("[MP3] Changed to folder "); Serial.println(mp3Folder);
#endif
    }
}

void playTrackFor(Color color) {
    Folder folder = (mp3Folder == MIXED) ? Folder(SENSOR_NO) : mp3Folder;
    Track track = mp3TrackMap[color];
#if DEBUG
    Serial.print("[MP3] Playing track "); Serial.print(folder);
    Serial.print("\\"); Serial.println(track);
#endif
    mp3.playFolderTrack(folder, track);
}

class Mp3Callbacks {
public:
    // See DfMp3_Error for code meaning
    static void OnError(DfMp3& mp3, uint16_t errorCode) {
        Serial.print("[MP3] Error "); Serial.println(errorCode);
    }

    static void OnPlayFinished(DfMp3& mp3, DfMp3_PlaySources source, uint16_t track) {
#if DEBUG
        Serial.print("[MP3] Finished playing "); Serial.println(track);
#endif
    }

    static void OnPlaySourceOnline(DfMp3& mp3, DfMp3_PlaySources source) {
#if DEBUG
        Serial.println("[MP3] Media online");
#endif
    }

    static void OnPlaySourceInserted(DfMp3& mp3, DfMp3_PlaySources source) {
#if DEBUG
        Serial.println("[MP3] Media inserted");
#endif
    }

    static void OnPlaySourceRemoved(DfMp3& mp3, DfMp3_PlaySources source) {
#if DEBUG
        Serial.println("[MP3] Media removed");
#endif
    }
};
