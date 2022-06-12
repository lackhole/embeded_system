//
// Created by YongGyu Lee on 2022/06/10.
//

#ifndef EMBED_NETWORK_ASYNC_VIDEO_CLIENT_H_
#define EMBED_NETWORK_ASYNC_VIDEO_CLIENT_H_

#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>
#include <fstream>


#include "opencv2/opencv.hpp"

#include "embed/utility/date_time.h"
#include "embed/utility/ring_buffer.h"
#include "embed/network/protocol.h"
#include "embed/network/tcp_client.h"

// TODO: Load from external file?
static constexpr const char* kHostURL = "lackhole.com";
static constexpr const int kHostPort = 7000;

class AsyncVideoClient {
 public:
  AsyncVideoClient(const std::string& url = kHostURL, int port = kHostPort)
    : AsyncVideoClient(url, std::to_string(port)) {}

  AsyncVideoClient(const std::string& url, const std::string& port)
    : client_(url, port)
  {
    thread_ = std::thread(&AsyncVideoClient::RunAsync, this);
  }

  ~AsyncVideoClient() { join(); }

  void feed(cv::Mat image, std::string timestamp) {
    input_.store(std::move(image), std::move(timestamp));
    run();
  }

  void run() {
    {
      std::lock_guard lck(mutex_);
      stop_ = false;
    }
    cv_.notify_all();
  }

  void stop() {
    {
      std::lock_guard lck(mutex_);
      stop_ = true;
    }
    cv_.notify_all();
  }

 private:
  void RunAsync() {
    std::unique_lock lck(mutex_);

    while(true) {
      cv_.wait(lck, [&]() {
        return !stop_ || terminate_;
      });

      if (terminate_)
        break;

      lck.unlock();
      const auto input = input_.load();
      if (input) {
        const auto& image = input->get().first;
        auto& timestamp = input->get().second;

        std::vector<uchar> buf;
        cv::imencode(".jpg", image, buf);

        try {
          protocol_.Post(
            client_,
            buf,
            Protocol::key_value_pair({
              {"Timestamp", std::move(timestamp)},
              {"FileFormat", ".jpg"}
            }));
        } catch (const std::exception& e) {
          std::cerr << e.what() << '\n';
        }
      }
      lck.lock();
      stop_ = true;
    }
  }

  void join() {
    terminate_ = true;
    cv_.notify_all();
    if (thread_.joinable()) {
      thread_.join();
    }
  }

  RingBuffer<std::pair<cv::Mat/*image*/, std::string/*timestamp*/>> input_{2};
  TcpClient client_;
  Protocol protocol_;

  std::thread thread_;
  bool stop_{true};
  bool terminate_{false};

  mutable std::mutex mutex_;
  std::condition_variable cv_;
};

#endif // EMBED_NETWORK_ASYNC_VIDEO_CLIENT_H_
