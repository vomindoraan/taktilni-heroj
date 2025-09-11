#pragma once

#define SERIAL_BAUD_RATE    115200
#define SW_SERIAL_BAUD_RATE 9600
#define SERIAL_BEGIN_DELAY  2500

#define CHANGE_MODE_CMD 'M'

#define ARRAY_LEN(a) (sizeof (a) / sizeof *(a))

#define WAIT_FOREVER() for (;;) delay(100)
