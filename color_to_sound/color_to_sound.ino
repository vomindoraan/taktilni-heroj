#include <Wire.h>
#include <Adafruit_TCS34725.h>
#include <DFMiniMp3.h>

#define DEBUG          2
#define WAIT_FOREVER() for (;;) delay(100)

#define COLOR_COUNT          7
#define COLOR_C_THRESHOLD    160
#define COLOR_DIST_THRESHOLD 10
// #define COLOR_HYSTERESIS     2

#define MP3_FOLDER    1   // 01 – low A, 02 – high A
#define MP3_VOLUME    30  // 0–30
#define TRACK_C_MAJOR 1
#define TRACK_D_MAJOR 2
#define TRACK_G_MAJOR 3
#define TRACK_D       4
#define TRACK_E       5
#define TRACK_F_SHARP 6
#define TRACK_A       7

enum Color : uint16_t {
    NONE      = UINT16_MAX,
    BLACK     = 0,
    RED       = 1,
    GREEN     = 2,
    BLUE      = 3,
    YELLOW    = 4,
    PURPLE    = 5,
    CYAN      = 6,
    ORANGE    = 7,
    PINK      = 8,
    LIGHTBLUE = 9,
    WHITE     = 10,
    _COUNT
};

uint16_t const COLOR_SAMPLES[COLOR_COUNT][5] = {
    { Color::RED,    128, 37,  26,  196 },
    { Color::GREEN,  55,  89,  48,  205 },
    { Color::BLUE,   30,  56,  75,  168 },
    { Color::YELLOW, 369, 233, 87,  744 },
    { Color::CYAN,   104, 221, 168, 492 },
    { Color::ORANGE, 332, 114, 76,  511 },
    { Color::PINK,   351, 277, 229, 904 },
};

Adafruit_TCS34725 tcs(TCS34725_INTEGRATIONTIME_101MS, TCS34725_GAIN_4X);

class Mp3Callbacks;
using DfMp3 = DFMiniMp3<HardwareSerial, Mp3Callbacks>;
DfMp3 mp3(Serial1);

uint8_t mp3TrackMap[Color::_COUNT];  // Color → track number

void setup() {
    mp3TrackMap[Color::RED]    = TRACK_C_MAJOR;
    mp3TrackMap[Color::GREEN]  = TRACK_G_MAJOR;
    mp3TrackMap[Color::BLUE]   = TRACK_D_MAJOR;
    mp3TrackMap[Color::YELLOW] = TRACK_E;
    mp3TrackMap[Color::CYAN]   = TRACK_F_SHARP;
    mp3TrackMap[Color::ORANGE] = TRACK_D;
    mp3TrackMap[Color::PINK]   = TRACK_A;

    Serial.begin(115200);

    if (!tcs.begin()) {
        Serial.println("[TCS] No sensor found");
        WAIT_FOREVER();
    }
#if DEBUG
    else {
        Serial.println("[TCS] Sensor connected");
    }
#endif

    mp3.begin();
#if DEBUG
    mp3.reset();
#endif
    mp3.setVolume(MP3_VOLUME);
}

void loop() {
    static Color prevColor = Color::NONE;
    // static Color playColor = Color::NONE;
    // static uint8_t hysteresisCnt = 0;

    uint16_t r, g, b, c;
    tcs.getRawData(&r, &g, &b, &c);

#if DEBUG >= 2
    // uint16_t temp = tcs.calculateColorTemperature_dn40(r, g, b, c);
    // uint16_t lux = tcs.calculateLux(r, g, b);

    Serial.print("[TCS] ");
    // Serial.print("Temp: "); Serial.print(temp); Serial.print("K - ");
    // Serial.print("Lux: "); Serial.print(lux); Serial.print(" - ");
    Serial.print("R: "); Serial.print(r); Serial.print(", ");
    Serial.print("G: "); Serial.print(g); Serial.print(", ");
    Serial.print("B: "); Serial.print(b); Serial.print(", ");
    Serial.print("C: "); Serial.print(c); Serial.println();
#endif

    Color color = identifyColor(r, g, b, c);
    if (color == Color::NONE) {
        prevColor = Color::NONE;
        // playColor = Color::NONE;
        // hysteresisCnt = 0;
        return;
    }
#if DEBUG
    Serial.print("[TCS] Identified color: "); Serial.println(color);
#endif

    // if (color != prevColor) {
    //     hysteresisCnt = 1;
    // } else if (++hysteresisCnt >= COLOR_HYSTERESIS && color != playColor) {
    //     playTrackFor(playColor = color);
    // }
    if (color != prevColor) {
        playTrackFor(color);
    }
    prevColor = color;
}

Color identifyColor(uint16_t r, uint16_t g, uint16_t b, uint16_t c) {
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
        uint16_t cDist = abs(static_cast<int>(c) - static_cast<int>(cSample));
#if DEBUG >= 2
        Serial.print(dist); Serial.print("/"); Serial.print(cDist);
        Serial.print((i < COLOR_COUNT-1) ? ", " : "\n");
#endif
        if (dist < COLOR_DIST_THRESHOLD && dist < bestMatchDist && cDist < bestMatchCDist) {
            bestMatch = static_cast<Color>(color);
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
    r = ((uint32_t) r << 8) / c;
    g = ((uint32_t) g << 8) / c;
    b = ((uint32_t) b << 8) / c;
    rSample = ((uint32_t) rSample << 8) / cSample;
    gSample = ((uint32_t) gSample << 8) / cSample;
    bSample = ((uint32_t) bSample << 8) / cSample;

    int rDiff = static_cast<int>(r) - static_cast<int>(rSample);
    int gDiff = static_cast<int>(g) - static_cast<int>(gSample);
    int bDiff = static_cast<int>(b) - static_cast<int>(bSample);
    return static_cast<uint16_t>(
        sqrt(pow(rDiff, 2) + pow(rDiff, 2) + pow(bDiff, 2))
    );
}

void playTrackFor(Color color) {
    uint8_t track = mp3TrackMap[color];
#if DEBUG
    Serial.print("[MP3] Playing track #"); Serial.print(track); Serial.println("...");
#endif
    mp3.playFolderTrack(MP3_FOLDER, track);
}

class Mp3Callbacks {
public:
    // See DfMp3_Error for code meaning
    static void OnError(DfMp3& mp3, uint16_t errorCode) {
        Serial.print("[MP3] Error "); Serial.println(errorCode);
    }

    static void OnPlayFinished(DfMp3& mp3, DfMp3_PlaySources source, uint16_t track) {
#if DEBUG
        Serial.print("[MP3] Finished playing #"); Serial.println(track);
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
