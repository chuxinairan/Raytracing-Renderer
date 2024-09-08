#pragma once

#include <atomic>
#include <functional>
#include <queue>
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
  std::atomic<int> pending_task_count;  // 记录未执行完成的任务数量
  std::queue<Task *> tasks;
  Spinlock spin_lock{};
};

extern ThreadPool thread_pool;