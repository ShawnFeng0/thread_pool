#pragma once

#include <functional>
#include <future>
#include <iostream>
#include <thread>

#include "mutex_queue.h"

class ThreadPool {
 public:
  explicit ThreadPool(
      size_t max_thread_number = std::thread::hardware_concurrency())
      : max_thread_number_(max_thread_number) {
    auto thread_task = [&]() {
      while (true) {
        std::packaged_task<void()> task;
        if (!task_queue_.pop_wait_if_empty(&task)) {
          break;
        }

        task();
      }
    };

    thread_pool_.reserve(max_thread_number);
    for (int i = 0; i < max_thread_number_; ++i) {
      thread_pool_.emplace_back(thread_task);
    }
  }

  ~ThreadPool() {
    task_queue_.alert_for_exit();
    for (auto& t : thread_pool_) {
      t.join();
    }
  }

  template <typename Fn, typename... Args>
  decltype(auto) enqueue(Fn&& fn, Args&&... args) {
    using ReturnType = decltype(fn(std::forward<Args>(args)...));

    auto task_ptr = std::make_unique<std::packaged_task<ReturnType()>>(
        std::bind(std::forward<Fn>(fn), std::forward<Args>(args)...));

    auto future = task_ptr->get_future();

    task_queue_.emplace_wait_if_full(
        [task_ptr = std::move(task_ptr)] { (*task_ptr)(); });

    return future;
  }

 private:
  std::vector<std::thread> thread_pool_;
  MutexQueue<std::packaged_task<void()>, 128> task_queue_;
  const size_t max_thread_number_;
};
