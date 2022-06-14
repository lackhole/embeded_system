//
// Created by YongGyu Lee on 2022/06/12.
//

#ifndef WATCHER_UTILITY_SYNC_STREAM_H_
#define WATCHER_UTILITY_SYNC_STREAM_H_

#include <ostream>
#include <sstream>
#include <type_traits>

namespace watcher {

class OSyncStream {
 public:
  template<typename OStream>
  OSyncStream(OStream& stream) : os_(stream) {}

  ~OSyncStream() {
    os_ << ss.str();
  }

  template<typename T>
  OSyncStream& operator<<(const T& val) {
    ss << val;
    return *this;
  }

 private:
  std::ostringstream ss;
  std::ostream& os_;
};

} // namespace watcher

#endif // WATCHER_UTILITY_SYNC_STREAM_H_
