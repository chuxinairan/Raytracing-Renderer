#pragma once

#include <atomic>
#include <functional>
#include <list>
#include <thread>
#include <vector>

#include "spin_lock.hpp"

class Task {
public:
  virtual void run() = 0;
};

class ThreadPool {
public:
  static void WorkerThread(ThreadPool *master);

  void wait() const;

  ThreadPool(size_t thread_count = 0);
  ~ThreadPool();

  void parallelFor(size_t width, size_t height,
                   const std::function<void(size_t, size_t)> &lambda);

  void addTask(Task *task);
  Task *getTask();

private:
  std::atomic<int> alive;
  std::vector<std::thread> threads;
  std::list<Task *> tasks;
  Spinlock spin_lock{};
};