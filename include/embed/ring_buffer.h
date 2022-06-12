//
// Created by YongGyu Lee on 2022/05/30.
//

#ifndef EMBED_RING_BUFFER_H_
#define EMBED_RING_BUFFER_H_

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

  void store(T val) {
    container_.emplace(container_.begin() + next_index(), std::move(val));
    next();
  }

  template<typename... U>
  std::enable_if_t<
    std::is_constructible_v<value_type, U&&...>
  >
  store(U&&... args) {
    container_.emplace(container_.begin() + next_index(), std::forward<U>(args)...);
    next();
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

  int next_index() const {
    return (idx_ + 1) % container_.size();
  }

 private:
  void next() {
    const auto sz = container_.size();
    const int prev = idx_;
    idx_ = (prev + 1) % sz;
  }

  container_type container_;
  std::atomic<int> idx_{-1};
};

#endif // EMBED_RING_BUFFER_H_
