#pragma once

using time_ms = unsigned long;

ISR(TIMER1_COMPA_vect);

class Timer1 {
public:
    static Timer1& instance() {
        static Timer1 i;
        return i;
    }

    void begin(time_ms period);
    bool ready();  // Read and reset flag

    friend void TIMER1_COMPA_vect(void);

private:
    bool volatile flag;

    Timer1() {}
};
