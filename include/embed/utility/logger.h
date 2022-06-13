//
// Created by YongGyu Lee on 2022/06/12.
//

#ifndef EMBED_UTILITY_LOGGER_H_
#define EMBED_UTILITY_LOGGER_H_

#include <iostream>
#include <sstream>
#include <utility>

#include "embed/utility/date_time.h"

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


#endif // EMBED_UTILITY_LOGGER_H_
