#pragma once

#define _STR(x)      #x
#define STR(x)       _STR(x)
#define ARRAY_LEN(a) (sizeof (a) / sizeof *(a))

#define WAIT_FOREVER() for (;;) delay(100UL)

#define SERIAL_BAUD_RATE    115200UL
#define SW_SERIAL_BAUD_RATE 9600UL
#define SERIAL_BEGIN_DELAY  2500UL

#define CMD_SYNC        'S'
#define CMD_CHANGE_MODE 'M'

#define SYNC_TIMEOUT     5000UL
#define SYNC_BPM_SLOW    60
#define SYNC_BPM_FAST    200
#define SYNC_PER_BEAT    2
#define SYNC_PERIOD_SLOW (60000UL / SYNC_BPM_SLOW / SYNC_PER_BEAT)
#define SYNC_PERIOD_FAST (60000UL / SYNC_BPM_FAST / SYNC_PER_BEAT)
#define SYNC_PERIOD_MEAN ((SYNC_PERIOD_SLOW + SYNC_PERIOD_FAST) / 2)

using time_ms = unsigned long;
