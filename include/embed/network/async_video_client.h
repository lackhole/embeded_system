//
// Created by YongGyu Lee on 2022/06/10.
//

#ifndef EMBED_NETWORK_ASYNC_VIDEO_CLIENT_H_
#define EMBED_NETWORK_ASYNC_VIDEO_CLIENT_H_

#include <string>
#include <fstream>


#include "opencv2/opencv.hpp"

#include "embed/utility/async_runner.h"
#include "embed/utility/date_time.h"
#include "embed/utility/ring_buffer.h"
#include "embed/network/protocol.h"
#include "embed/network/tcp_client.h"

class AsyncVideoClient {
 public:
  AsyncVideoClient(const std::string& url, const std::string& port)
    : client_(url, port)
  {
    async_runner_.AddWakeUpListener([this](){ OnWakeUp(); });
  }

  void feed(cv::Mat image, std::string timestamp);


 private:
  void OnWakeUp();

  RingBuffer<std::pair<cv::Mat/*image*/, std::string/*timestamp*/>> input_{2};
  TcpClient client_;
  Protocol protocol_;

  AsyncRunner async_runner_;
};

#endif // EMBED_NETWORK_ASYNC_VIDEO_CLIENT_H_
