//
// Created by YongGyu Lee on 2022/06/10.
//

#ifndef WATCHER_NETWORK_ASYNC_VIDEO_CLIENT_H_
#define WATCHER_NETWORK_ASYNC_VIDEO_CLIENT_H_

#include <fstream>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <unordered_map>
#include <vector>

#include "boost/signals2.hpp"
#include "opencv2/opencv.hpp"

#include "watcher/network/protocol.h"
#include "watcher/network/tcp_client.h"
#include "watcher/utility/async_runner.h"
#include "watcher/utility/ring_buffer.h"

namespace watcher {

class AsyncVideoClient {
 public:
  AsyncVideoClient(const std::string& url, const std::string& port)
    : client_(url, port)
  {
    conn_ = async_runner_.AddWakeUpListener([this](){ OnWakeUp(); });
  }

  void feed(cv::Mat image, std::string timestamp,
            std::vector<std::string> detected_object);

  template<typename F, typename ...Args,
    std::enable_if_t<
      std::is_invocable_v<F, std::unordered_map<std::string, std::string>>
    , int> = 0>
  boost::signals2::connection AddGetListener(F func, std::string request) {
    return get_listener_.connect(
      [f = std::move(func), r = std::move(request), this] () {
        try {
          auto response = protocol_.Get(client_, r);
          f(std::move(response));
        } catch (const std::exception& e) {
          Log.e(e.what());
        }
    });
  }

 private:
  void OnWakeUp();

  RingBuffer<std::tuple<cv::Mat/*image*/, std::string/*timestamp*/, std::vector<std::string>>> input_{2};
  TcpClient client_;
  Protocol protocol_;

  AsyncRunner async_runner_;
  boost::signals2::scoped_connection conn_;

  boost::signals2::signal<void()> get_listener_;
};

} // namespace watcher

#endif // WATCHER_NETWORK_ASYNC_VIDEO_CLIENT_H_
