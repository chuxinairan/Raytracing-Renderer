#pragma once

#include <atomic>
#include <thread>

class Spinlock {
public:
  void acquire() {
    while (flag.test_and_set(std::memory_order_acquire))
      std::this_thread::yield();
  }
  void release() { flag.clear(std::memory_order_release); }

private:
  std::atomic_flag flag{};
};

class Guard {
public:
  Guard(Spinlock &spin_lock) : spinLock(spin_lock) { spinLock.acquire(); }
  ~Guard() { spinLock.release(); }

private:
  Spinlock &spinLock;
};