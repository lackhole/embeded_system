//
// Created by YongGyu Lee on 2022/06/10.
//

#ifndef WATCHER_NETWORK_ASYNC_VIDEO_CLIENT_H_
#define WATCHER_NETWORK_ASYNC_VIDEO_CLIENT_H_

#include <fstream>
#include <string>
#include <tuple>
#include <vector>

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
    async_runner_.AddWakeUpListener([this](){ OnWakeUp(); });
  }

  void feed(cv::Mat image, std::string timestamp,
            std::vector<std::string> detected_object);


 private:
  void OnWakeUp();

  RingBuffer<std::tuple<cv::Mat/*image*/, std::string/*timestamp*/, std::vector<std::string>>> input_{2};
  TcpClient client_;
  Protocol protocol_;

  AsyncRunner async_runner_;
};

} // namespace watcher

#endif // WATCHER_NETWORK_ASYNC_VIDEO_CLIENT_H_
