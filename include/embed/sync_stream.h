//
// Created by YongGyu Lee on 2022/06/12.
//

#ifndef EMBED_SYNC_STREAM_H_
#define EMBED_SYNC_STREAM_H_

#include <ostream>
#include <sstream>
#include <type_traits>

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

#endif // EMBED_SYNC_STREAM_H_
