#include "counter.h"

Counter::Counter(PinName pin):_interrupt(pin)
{
	_interrupt.rise(this, &Counter::increment);
}

void Counter::increment()
{
	_count++;
}

int Counter::read()
{
	return _count;
}

void Counter::write(int new_count)
{
	_count = new_count;
}
