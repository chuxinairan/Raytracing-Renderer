#pragma once

#include <atomic>
#include <functional>
#include <queue>
#include <thread>
#include <vector>

#include "spin_lock.hpp"

class Task {
public:
  virtual ~Task() = default;
  virtual void run() = 0;
};

class ThreadPool {
public:
  static void WorkerThread(ThreadPool *master);

  ThreadPool(size_t thread_count = 0);
  ~ThreadPool();

  void parallelFor(size_t width, size_t height, const std::function<void(size_t, size_t)> &lambda, bool isComplex = true);
  void wait() const;

  void addTask(Task *task);
  Task *getTask();

private:
  std::atomic<int> alive;
  std::vector<std::thread> threads;
  std::atomic<int> pending_task_count;
  std::queue<Task *> tasks;
  Spinlock spin_lock{};
};

extern ThreadPool thread_pool;