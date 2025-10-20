#pragma once

ISR(TIMER1_COMPA_vect);

class Timer1 {
public:
    static Timer1& instance() {
        static Timer1 i;
        return i;
    }

    void begin(unsigned long intervalMs);
    bool ready();

    friend void TIMER1_COMPA_vect(void);

private:
    volatile bool flag;

    Timer1() {}
};
