//
// Created by YongGyu Lee on 2022/06/12.
//

#ifndef EMBED_UTILITY_ASYNC_RUNNER_H_
#define EMBED_UTILITY_ASYNC_RUNNER_H_

#include <condition_variable>
#include <mutex>
#include <thread>
#include <type_traits>

#include "boost/signals2.hpp"

enum class async_run {
  now,
  deferred,
};

class AsyncRunner {
 public:
  explicit AsyncRunner(bool always_run = false,
                       async_run run_mode = async_run::deferred)
    : always_run_(always_run), stop_(run_mode == async_run::deferred)
  {
    thread_ = std::thread(&AsyncRunner::RunAsync, this);
  }

  ~AsyncRunner() { join(); }

  template<typename F>
  boost::signals2::connection AddWakeUpListener(F func) {
    return on_wakeup_.connect(std::move(func));
  }

  void run() {
    {
      std::lock_guard lck(mutex_);
      stop_ = false;
    }
    cv_.notify_all();
  }

  void stop() {
    {
      std::lock_guard lck(mutex_);
      stop_ = true;
    }
    cv_.notify_all();
  }

 private:
  void RunAsync() {
    std::unique_lock lck(mutex_);

    while(true) {
      cv_.wait(lck, [&]() {
        return !stop_ || terminate_;
      });

      if (terminate_)
        break;

      lck.unlock();
      on_wakeup_();
      lck.lock();
      stop_ = !always_run_;
    }
  }

  void join() {
    terminate_ = true;
    cv_.notify_all();
    if (thread_.joinable()) {
      thread_.join();
    }
  }

  std::thread thread_;
  bool stop_;
  bool terminate_{false};
  bool always_run_;

  mutable std::mutex mutex_;
  std::condition_variable cv_;

  boost::signals2::signal<void()> on_wakeup_;
};

#endif // EMBED_UTILITY_ASYNC_RUNNER_H_
