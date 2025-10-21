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

// TODO: Determine best values
#define SYNC_PERIOD_SLOW 500UL
#define SYNC_PERIOD_FAST 150UL
#define SYNC_TIMEOUT     5000UL

using time_ms = unsigned long;
