//
// Created by YongGyu Lee on 2022/05/30.
//

#ifndef EMBED_UTILITY_RING_BUFFER_H_
#define EMBED_UTILITY_RING_BUFFER_H_

#include <atomic>
#include <optional>
#include <type_traits>
#include <vector>

template<typename T, typename Container = std::vector<T>>
class RingBuffer {
 public:
  using value_type = T;
  using container_type = Container;

  RingBuffer(size_t size = 2) : container_{size} {}

  template<typename... U>
  std::enable_if_t<
    std::is_constructible_v<value_type, U&&...>
  >
  store(U&&... args) {
    container_.emplace(container_.begin() + next(), std::forward<U>(args)...);
  }

  std::optional<std::reference_wrapper<const value_type>> load() const {
    if (idx_ == -1)
      return std::nullopt;
    return container_[idx_];
  }
  std::optional<std::reference_wrapper<value_type>> load() {
    if (idx_ == -1)
      return std::nullopt;
    return container_[idx_];
  }

  void reset() {
    idx_ = -1;
  }

 private:
  int next() {
    const auto sz = container_.size();
    const int prev = idx_;
    const auto new_sz = (prev + 1) % sz;
    idx_ = new_sz;
    return new_sz;
  }

  container_type container_;
  std::atomic<int> idx_{-1};
};

#endif // EMBED_UTILITY_RING_BUFFER_H_
