#pragma once

#define STR(x)    _STR(x)
#define _STR(x)   #x
#define ARRLEN(a) (sizeof (a) / sizeof *(a))

#define WAIT_FOREVER() for (;;) delay(100UL)

#define SERIAL_BAUD_RATE    115200UL
#define SW_SERIAL_BAUD_RATE 9600UL
#define SERIAL_BEGIN_DELAY  2500UL

#define CMD_SYNC        'S'
#define CMD_CHANGE_MODE 'M'

#define SYNC_TIMEOUT     1500UL
#define SYNC_PERIOD_LOW  235UL  // 1/8 notes (fastest)
#define SYNC_PERIOD_HIGH 615UL  // 1/8 notes (slowest)

#define BPM(p) (30000UL / (p))  // 4/4 time

using time_ms = unsigned long;
using mode_t  = byte;
