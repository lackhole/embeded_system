//
// Created by YongGyu Lee on 2022/06/12.
//

#ifndef WATCHER_UTILITY_LOGGER_H_
#define WATCHER_UTILITY_LOGGER_H_

#include <chrono>
#include <iostream>
#include <sstream>

#include "watcher/utility/date_time.h"

namespace watcher {

class Log_t {
 public:
  constexpr Log_t() = default;

  template<typename ...Args> void d(const Args&... args) const { write(std::cout, args...); }
  template<typename ...Args> void e(const Args&... args) const { write(std::cerr, args...); }

 private:
  template<typename OStream, typename ...Args>
  static void write(OStream& os, const Args&... args) {
    std::stringstream ss;
    ss << DateTime<>::now(std::chrono::hours(9)).to_string() << " | ";
    (ss << ... << args);
    ss << '\n';
    os << ss.str();
  }
};

constexpr inline Log_t Log{};

} // namespace watcher

#endif // WATCHER_UTILITY_LOGGER_H_
