//
// Created by YongGyu Lee on 2022/05/07.
//

#ifndef EMBED_UTILITY_DATE_TIME_H_
#define EMBED_UTILITY_DATE_TIME_H_

#include <chrono>
#include <ostream>
#include <string>
#include <utility>


template<typename ...Args>
std::string format_string(const char* fmt, const Args&... val) {
  std::string buffer(100, '\0');
  const auto size = snprintf(NULL, 0, fmt, val...);
  std::snprintf(const_cast<char*>(buffer.data()), size, fmt, val...);
  buffer.resize(size - 1);
  return buffer;
}

template<typename Clock = std::chrono::system_clock>
class DateTime {
  using days = std::chrono::duration<int, std::ratio_multiply<std::chrono::hours::period, std::ratio<24>>::type>;
 public:
  using clock = Clock;
  using time_point = typename clock::time_point;
  using duration = typename clock::duration;

  using milliseconds_type = std::chrono::milliseconds;
  using seconds_type = std::chrono::seconds;

  constexpr explicit DateTime(time_point tp, duration time_zone = std::chrono::hours(0))
    : now_(tp.time_since_epoch() + time_zone), time_zone_(time_zone)
  {}

  DateTime& format(const char* fmt) { format_ = fmt; return *this; }
  DateTime& format(std::string fmt) { format_ = std::move(fmt); return *this; }
  [[nodiscard]] const std::string& format() const { return format_; }

  DateTime& time_zone(duration tz) {
    now_ += (tz - time_zone_);
    time_zone_ = std::move(tz);
    return *this;
  }
  [[nodiscard]] const duration& time_zone() const { return time_zone_; }

  [[nodiscard]] auto ymd() const noexcept {
    static_assert(
      std::numeric_limits<unsigned>::digits >= 18,
      "This algorithm has not been ported to a 16 bit unsigned integer");
    static_assert(
      std::numeric_limits<int>::digits >= 20,
      "This algorithm has not been ported to a 16 bit signed integer");

    auto z = std::chrono::duration_cast<days>(now_).count();
    z += 719468;
    const int era = (z >= 0 ? z : z - 146096) / 146097;
    const unsigned doe = static_cast<unsigned>(z - era * 146097); // [0, 146096]
    const unsigned yoe =
      (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;      // [0, 399]
    const int y = static_cast<int>(yoe) + era * 400;
    const unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100); // [0, 365]
    const unsigned mp = (5 * doy + 2) / 153;                      // [0, 11]
    const unsigned d = doy - (153 * mp + 2) / 5 + 1;              // [1, 31]
    const unsigned m = (mp < 10 ? mp + 3 : mp - 9);               // [1, 12]
    return std::make_tuple(y + (m <= 2), m, d);
  }

  [[nodiscard]] std::string to_string() const {
    namespace chrono = std::chrono;

    auto time_point = now_;

    days d = chrono::duration_cast<days>(time_point);
    time_point -= d;
    chrono::hours h = chrono::duration_cast<chrono::hours>(time_point);
    time_point -= h;
    chrono::minutes m = chrono::duration_cast<chrono::minutes>(time_point);
    time_point -= m;
    chrono::seconds s = chrono::duration_cast<chrono::seconds>(time_point);
    time_point -= s;
    chrono::milliseconds ms = chrono::duration_cast<chrono::milliseconds>(time_point);
    time_point -= ms;

    const auto date = ymd(); // assumes that system_clock uses
    // 1970-01-01 0:0:0 UTC as the epoch,
    // and does not count leap seconds.

    const auto yr = std::get<0>(date);
    const auto mt = std::get<1>(date);
    const auto dy = std::get<2>(date);

    return format_string(format_.c_str(),
                         static_cast<int>(yr), static_cast<int>(mt), static_cast<int>(dy),
                         static_cast<int>(h.count()), static_cast<int>(m.count()),
                         static_cast<int>(s.count()), static_cast<int>(ms.count()));
  }

  static DateTime now(duration time_zone = std::chrono::hours(0)) {
    return DateTime(clock::now(), std::move(time_zone));
  }

  [[nodiscard]] int64_t milliseconds() const { return cast_to<milliseconds_type>().count(); }
  [[nodiscard]] int64_t seconds()      const { return cast_to<seconds_type>().count(); }

  template<typename Duration>
  Duration cast_to() const {
    return std::chrono::duration_cast<Duration>(now_);
  }

 private:
  duration time_zone_;
  duration now_;
  std::string format_ = "%04d-%02d-%02d-%02d:%02d:%02d:%03d";
};

template<typename Clock>
std::ostream& operator<<(std::ostream& os, const DateTime<Clock>& dt) {
  os << dt.to_string();
  return os;
}

#endif // EMBED_UTILITY_DATE_TIME_H_
