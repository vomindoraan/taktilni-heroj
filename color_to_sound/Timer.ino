#include "Timer.h"

void Timer1::begin(time_ms interval) {
    uint16_t ocr = F_CPU / 256 * interval / 1000 - 1;
    noInterrupts();
    flag = false;
    TCCR1A = TCCR1B = 0;      // Clear control registers
    TCNT1 = 0;                // Reset counter
    OCR1A = ocr;              // Set compare-match register
    TCCR1B |= (1 << WGM12);   // CTC mode
    TCCR1B |= (1 << CS12);    // Prescaler = 256
    TIMSK1 |= (1 << OCIE1A);  // Enable Timer1 compare interrupt
    interrupts();
}

bool Timer1::ready() {
    bool ret = flag;
    flag = false;
    return ret;
}

ISR(TIMER1_COMPA_vect) {
    Timer1::instance().flag = true;
}
