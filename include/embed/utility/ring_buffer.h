//
// Created by YongGyu Lee on 2022/05/30.
//

#ifndef EMBED_UTILITY_RING_BUFFER_H_
#define EMBED_UTILITY_RING_BUFFER_H_

#include <mutex>
#include <optional>
#include <type_traits>
#include <vector>

template<typename T, typename Container = std::vector<std::optional<T>>>
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
    std::lock_guard lck(m_);
    container_.emplace(container_.begin() + next(), value_type(std::forward<U>(args)...));
  }

  void store(std::nullopt_t) {
    std::lock_guard lck(m_);
    container_.emplace(container_.begin() + next(), std::nullopt);
  }

  void store(std::optional<value_type> o) {
    std::lock_guard lck(m_);
    container_.emplace(container_.begin() + next(), std::move(o));
  }

  std::optional<const value_type> load() const {
    std::lock_guard lck(m_);
    if (idx_ == -1)
      return std::nullopt;
    return std::move(container_[idx_]);
  }

  std::optional<value_type> load() {
    std::lock_guard lck(m_);
    if (idx_ == -1)
      return std::nullopt;
    return std::move(container_[idx_]);
  }

//  std::optional<const value_type> load() const && {
//    std::lock_guard lck(m_);
//    if (idx_ == -1)
//      return std::nullopt;
//    return std::move(container_[idx_]);
//  }
//
//  std::optional<value_type> load() && {
//    std::lock_guard lck(m_);
//    if (idx_ == -1)
//      return std::nullopt;
//    return std::move(container_[idx_]);
//  }

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
  int idx_{-1};
  mutable std::mutex m_;
};

#endif // EMBED_UTILITY_RING_BUFFER_H_
