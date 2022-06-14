//
// Created by YongGyu Lee on 2022/06/12.
//

#include "watcher/utility/async_runner.h"

#include <mutex>
#include <thread>

namespace watcher {

AsyncRunner::AsyncRunner(bool always_run, async_run run_mode)
  : always_run_(always_run), stop_(run_mode == async_run::deferred)
{
  thread_ = std::thread(&AsyncRunner::RunAsync, this);
}

AsyncRunner::~AsyncRunner() { join(); }

void AsyncRunner::run() {
  {
    std::lock_guard lck(mutex_);
    stop_ = false;
  }
  cv_.notify_all();
}

void AsyncRunner::stop() {
  {
    std::lock_guard lck(mutex_);
    stop_ = true;
  }
  cv_.notify_all();
}

void AsyncRunner::RunAsync() {
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

void AsyncRunner::join() {
  terminate_ = true;
  cv_.notify_all();
  if (thread_.joinable()) {
    thread_.join();
  }
}

} // namespace watcher
