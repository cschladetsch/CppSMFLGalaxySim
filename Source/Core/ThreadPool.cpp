#include "Core/ThreadPool.hpp"
#include <spdlog/spdlog.h>

namespace Core {

ThreadPool::ThreadPool(std::size_t numThreads)
    : workers_(), tasks_(), queueMutex_(), condition_(), finished_(),
      stopping_(false), activeTasks_(0) {

  workers_.reserve(numThreads);

  for (std::size_t i = 0; i < numThreads; ++i) {
    workers_.emplace_back([this] { WorkerThread(); });
  }

  spdlog::info("ThreadPool created with {} threads", numThreads);
}

ThreadPool::~ThreadPool() {
  {
    std::unique_lock<std::mutex> lock(queueMutex_);
    stopping_ = true;
  }

  condition_.notify_all();

  for (auto &worker : workers_) {
    if (worker.joinable()) {
      worker.join();
    }
  }

  spdlog::info("ThreadPool destroyed");
}

void ThreadPool::WorkerThread() {
  while (true) {
    std::function<void()> task;

    {
      std::unique_lock<std::mutex> lock(queueMutex_);

      condition_.wait(lock, [this] { return stopping_ || !tasks_.empty(); });

      if (stopping_ && tasks_.empty()) {
        return;
      }

      task = std::move(tasks_.front());
      tasks_.pop();
      ++activeTasks_;
    }

    task();

    {
      std::unique_lock<std::mutex> lock(queueMutex_);
      --activeTasks_;

      if (tasks_.empty() && activeTasks_ == 0) {
        finished_.notify_all();
      }
    }
  }
}

void ThreadPool::WaitForAll() {
  std::unique_lock<std::mutex> lock(queueMutex_);
  finished_.wait(lock, [this] { return tasks_.empty() && activeTasks_ == 0; });
}

std::size_t ThreadPool::GetNumPendingTasks() const {
  std::unique_lock<std::mutex> lock(queueMutex_);
  return tasks_.size() + activeTasks_;
}

} // namespace Core