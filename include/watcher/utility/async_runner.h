//
// Created by YongGyu Lee on 2022/06/12.
//

#ifndef WATCHER_UTILITY_ASYNC_RUNNER_H_
#define WATCHER_UTILITY_ASYNC_RUNNER_H_

#include <condition_variable>
#include <mutex>
#include <thread>
#include <type_traits>

#include "boost/signals2.hpp"

namespace watcher {

enum class async_run {
  now,
  deferred,
};

class AsyncRunner {
 public:
  explicit AsyncRunner(bool always_run = false,
                       async_run run_mode = async_run::deferred);

  ~AsyncRunner();

  template<typename F>
  boost::signals2::connection AddWakeUpListener(F func) {
    return on_wakeup_.connect(std::move(func));
  }

  void run();

  void stop();

 private:
  void RunAsync();

  void join();

  std::thread thread_;
  bool stop_;
  bool terminate_{false};
  bool always_run_;

  mutable std::mutex mutex_;
  std::condition_variable cv_;

  boost::signals2::signal<void()> on_wakeup_;
};

} // namespace watcher

#endif // WATCHER_UTILITY_ASYNC_RUNNER_H_
