#ifndef COUNTER_H
#define COUNTER_H

#include "mbed.h"

class Counter
{
private:
    InterruptIn _interrupt;
    volatile int _count;

public:
    Counter(PinName pin);
    void increment();
    int read();
    void write(int new_count);
};

#endif
