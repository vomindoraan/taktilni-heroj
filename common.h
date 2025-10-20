#pragma once

#define _STR(x)      #x
#define STR(x)       _STR(x)
#define ARRAY_LEN(a) (sizeof (a) / sizeof *(a))

#define WAIT_FOREVER() for (;;) delay(100UL)

#define SERIAL_BAUD_RATE    115200UL
#define SW_SERIAL_BAUD_RATE 9600UL
#define SERIAL_BEGIN_DELAY  2500UL

#define CMD_SYNC            'S'
#define CMD_SYNC_TIMEOUT    5000UL
#define CMD_CHANGE_MODE     'M'
#define CMD_CHANGE_MODE_FMT "M%d"
#define CMD_CHANGE_MODE_SZ  8U

// TODO: Determine best values
#define SYNC_INTERVAL_SLOW  500UL
#define SYNC_INTERVAL_FAST  150UL

using time_ms = unsigned long;
