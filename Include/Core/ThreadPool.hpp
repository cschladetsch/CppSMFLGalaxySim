#pragma once

#include <concepts>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <type_traits>
#include <vector>

namespace Core {

class ThreadPool {
public:
  explicit ThreadPool(
      std::size_t numThreads = std::thread::hardware_concurrency());
  ~ThreadPool();

  ThreadPool(const ThreadPool &) = delete;
  ThreadPool &operator=(const ThreadPool &) = delete;
  ThreadPool(ThreadPool &&) = delete;
  ThreadPool &operator=(ThreadPool &&) = delete;

  template <typename F, typename... Args>
    requires std::invocable<F, Args...>
  auto Submit(F &&func,
              Args &&...args) -> std::future<std::invoke_result_t<F, Args...>> {
    using return_type = std::invoke_result_t<F, Args...>;

    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(func), std::forward<Args>(args)...));

    std::future<return_type> result = task->get_future();

    {
      std::unique_lock<std::mutex> lock(queueMutex_);

      if (stopping_) {
        throw std::runtime_error("Submit on stopped ThreadPool");
      }

      tasks_.emplace([task]() { (*task)(); });
    }

    condition_.notify_one();
    return result;
  }

  template <typename Container, typename F>
    requires std::ranges::range<Container> &&
             std::invocable<F, typename Container::value_type &>
  void ParallelForEach(Container &container, F &&func) {
    std::vector<std::future<void>> futures;
    futures.reserve(container.size());

    for (auto &item : container) {
      futures.push_back(Submit(std::forward<F>(func), std::ref(item)));
    }

    for (auto &future : futures) {
      future.wait();
    }
  }

  void WaitForAll();
  [[nodiscard]] std::size_t GetNumThreads() const noexcept {
    return workers_.size();
  }
  [[nodiscard]] std::size_t GetNumPendingTasks() const;

private:
  void WorkerThread();

private:
  std::vector<std::jthread> workers_;
  std::queue<std::function<void()>> tasks_;

  mutable std::mutex queueMutex_;
  std::condition_variable condition_;
  std::condition_variable finished_;

  std::atomic<bool> stopping_{false};
  std::atomic<std::size_t> activeTasks_{0};
};

} // namespace Core