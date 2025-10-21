#include "Timer.h"

void Timer1::begin(time_ms period) {
    uint16_t ocr = F_CPU / 256 * period / 1000 - 1;
    noInterrupts();
    flag = false;
    TCCR1A = TCCR1B = 0;     // Clear control registers
    TCNT1 = 0;               // Reset counter
    OCR1A = ocr;             // Set compare-match register
    bitSet(TCCR1B, WGM12);   // CTC mode
    bitSet(TCCR1B, CS12);    // Prescaler = 256
    bitSet(TIMSK1, OCIE1A);  // Enable Timer1 compare interrupt
    interrupts();
}

bool Timer1::ready() {
    noInterrupts();
    bool flagValue = flag;
    flag = false;
    interrupts();
    return flagValue;
}

ISR(TIMER1_COMPA_vect) {
    Timer1::instance().flag = true;
}
