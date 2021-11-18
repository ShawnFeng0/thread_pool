//
// Created by shawnfeng on 2021-08-12.
//

#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>

/**
 * Multi-threaded queue, use mutex to protect the queue
 * @tparam T Type
 * @tparam kMaxSize Maximum queue length
 *
 * Example:
 * @code{.cpp}
  auto int_queue = std::make_shared<MutexQueue<int, 100>>();

  // Producer
  std::thread{[=]() {
    for (int i = 0; i < 1024; i++) {
      int_queue->emplace_overwrite_oldest(i);
    }
    int_queue->alert_for_exit();
  }}.join();

  // Consumer
  std::thread{[=]() {
    while (true) {
      int value;

      // If the waiting fails, it means that the queue has been marked for
      // cancellation, and it should be disconnected and exited from
      // consumption
      if (!int_queue->pop_wait_if_empty(&value)) {
        std::cout << "The queue has ended." << std::endl;
        break;
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(10));

      // Otherwise, the data has been fetched from the queue
      std::cout << value << std::endl;
    }
  }}.detach();
 * @endcode
 *
 */
template <typename T, size_t kMaxSize>
class MutexQueue {
  static_assert(kMaxSize > 0, "max_size must be > 0");

 public:
  using value_type = T;

  auto size() {
    std::unique_lock<std::mutex> lock(mutex_);
    return queue_.size();
  };

  auto empty() {
    std::unique_lock<std::mutex> lock(mutex_);
    return queue_.size();
  };

  template <typename... Args>
  bool emplace_wait_if_full(Args &&...args) {
    std::unique_lock<std::mutex> lg(mutex_);

    not_full_notifier_.wait(lg,
                            [this] { return !is_full() || alert_for_exit_; });

    if (!is_full()) {
      queue_.emplace(std::forward<Args>(args)...);

      not_empty_notifier_.notify_all();
      return true;

    } else {
      /// For alert_for_exit_
      return false;
    }
  }

  template <typename... Args>
  void emplace_overwrite_oldest(Args &&...args) {
    std::unique_lock<std::mutex> lock(mutex_);

    while (is_full()) {
      queue_.pop();
    }

    queue_.emplace(std::forward<Args>(args)...);

    not_empty_notifier_.notify_all();
  }

  bool pop_if_not_empty(T *value) {
    std::unique_lock<std::mutex> lock(mutex_);

    if (is_empty()) {
      return false;
    }

    if (value) {
      *value = queue_.front();
    }
    queue_.pop();

    not_full_notifier_.notify_all();

    return true;
  }

  /**
   * Wait until there is data or an alert
   * @param value Data pointer to be filled in the queue
   * @return true Wait for the end, data is available
   * @return false The queue publisher has issued an alert and may need to end
   */
  bool pop_wait_if_empty(T *value) {
    std::unique_lock<std::mutex> lock(mutex_);

    /// If the queue is empty, wait until the queue is not empty
    not_empty_notifier_.wait(lock,
                             [this] { return !is_empty() || alert_for_exit_; });

    if (!is_empty()) {
      if (value) {
        *value = std::move(queue_.front());
      }
      queue_.pop();

      not_full_notifier_.notify_all();

      return true;

    } else {
      /// For alert_for_exit_
      return false;
    }
  }

  bool is_alert() { return alert_for_exit_; }

  /**
   * Use condition variables to notify threads blocked by the queue that the
   * queue has ended
   */
  void alert_for_exit() {
    std::unique_lock<std::mutex> lock(mutex_);
    alert_for_exit_ = true;
    not_empty_notifier_.notify_all();
  }

 private:
  bool is_full() { return queue_.size() >= kMaxSize; }
  bool is_empty() { return queue_.empty(); }

  std::queue<T> queue_;
  std::mutex mutex_;

  /**
   * @see alert_for_exit
   */
  std::atomic<bool> alert_for_exit_{false};

  /// When data is written, the queue is not empty and a notification is sent
  std::condition_variable not_empty_notifier_;

  /// Notify when the queue data is consumed and the queue changes from full to
  /// non-full
  std::condition_variable not_full_notifier_;
};
