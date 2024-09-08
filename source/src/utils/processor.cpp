#include "utils/processor.hpp"

Processor::Processor(size_t total, size_t step)
    : total(total), current(0), percentage(0), last_percentage(0), step(step) 
{
  std::cout << "0%" << std::endl;
}

void Processor::update(size_t count) 
{
  Guard guard(spin_lock);
  current += count;
  percentage = static_cast<float>(current) / static_cast<float>(total) * 100;
  if (percentage - last_percentage >= step) {
    std::cout << percentage << "%" << std::endl;
    last_percentage = percentage;
  }
}