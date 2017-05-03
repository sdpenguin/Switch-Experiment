#include "mbed.h"

class Counter {
public:
    Counter(PinName pin) : _interrupt(pin) {        // create the InterruptIn on the pin specified to Counter
        _interrupt.rise(this, &Counter::increment); // attach increment function of this counter instance
    }
 
    void increment() {
        _count++;
    }
 
    int read() {
        return _count;
    }

    void write(int new_count) {
        _count = new_count;
    }
 
private:
    InterruptIn _interrupt;
    volatile int _count;
};
