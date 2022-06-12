//
// Created by YongGyu Lee on 2022/06/12.
//

#ifndef EMBED_UTILITY_LOGGER_H_
#define EMBED_UTILITY_LOGGER_H_

#include <condition_variable>
#include <iostream>
#include <list>
#include <mutex>
#include <ostream>
#include <sstream>
#include <thread>
#include <utility>

class WriteThread {
 public:
  explicit WriteThread(std::ostream& os)
    : os_(os) {
    thread_ = std::thread(&WriteThread::work, this);
  }

  template<typename ...Args>
  void write(const Args&... args) {
    {
      std::lock_guard<std::mutex> lck(que_mutex_);
      if (exit_)
        return;

      std::stringstream ss;
      (ss << ... << args);
      jobs_.emplace_back([&os = os_, s = ss.str()](){
        os << s + '\n';
      });
    }
    {
      std::lock_guard<std::mutex> lck(wait_mutex_);
      job_ready_ = true;
    }
    wait_cv_.notify_all();
  }

  void stop() {
    {
      std::lock_guard<std::mutex> lck(que_mutex_);
      exit_ = true;
    }
    wait_cv_.notify_all();
    if (thread_.joinable())
      thread_.join();
  }

 private:
  void work() {
    std::unique_lock<std::mutex> lck_wait(wait_mutex_, std::defer_lock);
    std::unique_lock<std::mutex> lck_modify(que_mutex_, std::defer_lock);

    for (;;) {
      lck_wait.lock();
      wait_cv_.wait(lck_wait, [this]{
        return job_ready_ || exit_;
      });

      lck_modify.lock();
      auto it = jobs_.begin();
      auto end = jobs_.end();

      job_ready_ = false;
      lck_wait.unlock();

      for (;it != end;) {
        lck_modify.unlock();
        (*it)();
        lck_modify.lock();
        it = jobs_.erase(it);
      }

      if (exit_ && jobs_.empty()) {
        break;
      }
      lck_modify.unlock();
    }
  }

  std::ostream& os_;

  std::thread thread_;
  mutable std::mutex wait_mutex_;
  std::condition_variable wait_cv_;
  bool exit_ = false;

  std::list<std::function<void()>> jobs_;
  mutable std::mutex que_mutex_;
  bool job_ready_ = false;
};


class Logger {
 public:
  Logger(Logger const &) = delete;
  Logger(Logger &&) = delete;
  Logger& operator=(Logger const &) = delete;
  Logger& operator=(Logger &&) = delete;

  static Logger& getInstance() {
    static auto inst = new Logger();
    return *inst;
  }

  void join() {
    stdout_.stop();
    stderr_.stop();
  }

  template<typename ...Args>
  void out(const Args&... args) {
    stdout_.write(args...);
  }

  template<typename ...Args>
  void err(const Args&... args) {
    stderr_.write(args...);
  }

 private:
  Logger() = default;
  WriteThread stdout_{std::cout};
  WriteThread stderr_{std::cerr};
};

class Log_t {
 public:
  constexpr Log_t() = default;

  template<typename ...Args> void d(const Args&... args) const { Logger::getInstance().out(args...); }
  template<typename ...Args> void e(const Args&... args) const { Logger::getInstance().err(args...); }

  void join() const { Logger::getInstance().join(); }
};

constexpr Log_t Log{};


#endif // EMBED_UTILITY_LOGGER_H_
