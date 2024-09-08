#pragma once

#include <iostream>

#include "thread/spin_lock.hpp"

class Processor
{
public:
    Processor(size_t total, size_t step = 1);

    void update(size_t count);
private:
    size_t total, current;
    size_t percentage, last_percentage, step;
    Spinlock spin_lock;
};