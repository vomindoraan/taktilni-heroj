#pragma once

#define _STR(x)      #x
#define STR(x)       _STR(x)
#define ARRAY_LEN(a) (sizeof (a) / sizeof *(a))

#define WAIT_FOREVER() for (;;) delay(100)

#define SERIAL_BAUD_RATE    115200
#define SW_SERIAL_BAUD_RATE 9600
#define SERIAL_BEGIN_DELAY  2500

#define CMD_SYNC            'S'
#define CMD_CHANGE_MODE     'M'
#define CMD_CHANGE_MODE_FMT "M%d"
#define CMD_CHANGE_MODE_SZ  8
