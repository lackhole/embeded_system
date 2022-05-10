#ifndef EMBED_FREQUENCY_H_
#define EMBED_FREQUENCY_H_

#include <chrono>
#include <deque>
#include <type_traits>
#include <utility>

template<typename Duration = std::chrono::seconds,
  typename Clock = std::chrono::steady_clock,
  typename TimePoint = std::chrono::time_point<Clock>>
class Frequency {
 public:
  using duration = Duration;
  using rep = typename duration::rep;
  using period = typename duration::period;
  using clock = Clock;
  using time_point = TimePoint;

  Frequency()
    : duration_(1) {}

  explicit Frequency(const rep& r)
    : duration_(r) {}

  explicit Frequency(const duration& duration)
    : duration_(duration) {}

  template<typename Rep, typename Period, template<typename, typename> class Duration2,
    std::enable_if_t<
      std::is_constructible<duration, Duration2<Rep, Period>>::value,
      int> = 0>
  explicit Frequency(const Duration2<Rep, Period>& d)
    : duration_(d) {}

  // record time
  void tick(time_point tp = clock::now()) {
    emplace_back(std::move(tp));
  }

  // return array size
  template<typename T = int>
  T size() const {
    static_assert(std::is_arithmetic<T>::value, "T must be arithmetic type");
    return static_cast<T>(list_.size());
  }

  // get frequency
  template<typename T = int, typename D = duration>
  T freq() const {
    static_assert(std::is_arithmetic<T>::value, "T must be arithmetic type");
    if (list_.empty())
      return static_cast<T>(0);
    return size<T>() / static_cast<T>(std::chrono::duration_cast<D>(duration_).count());
  }

  // utility: check if callees time gap has elapsed the duration
  bool elapsed() const {
    const auto now_t = clock::now();
    if (timestamp_ + duration_ < now_t) {
      timestamp_ = now_t;
      return true;
    }
    return false;
  }

 private:
  template<typename Tp = time_point>
  void emplace_back(Tp&& tp) {
    list_.emplace_back(std::forward<Tp>(tp));
    removeExpired();
  }

  // list_.empty() == false must be guaranteed
  void removeExpired() {
    const auto& last_t = list_.back();

    auto it = list_.begin();
    while (it != list_.end() && *it < (last_t - duration_)) {
      ++it;
    }
    list_.erase(list_.begin(), it);
  }

  std::deque<time_point> list_;
  duration duration_;
  mutable time_point timestamp_{};
};

#endif // EMBED_FREQUENCY_H_
