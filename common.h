#pragma once

#define SERIAL_BAUD_RATE    115200
#define SW_SERIAL_BAUD_RATE 38400

#define CHANGE_MODE_CMD 'M'

#define ARRAY_LEN(a) (sizeof (a) / sizeof *(a))

#define WAIT_FOREVER() for (;;) delay(100)
