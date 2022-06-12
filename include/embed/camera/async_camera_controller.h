//
// Created by YongGyu Lee on 2022/05/10.
//

#ifndef EMBED_CAMERA_ASYNC_CAMERA_CONTROLLER_H_
#define EMBED_CAMERA_ASYNC_CAMERA_CONTROLLER_H_

#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

#include "opencv2/opencv.hpp"

#include "cross_camera.h"
#include "embed/utility/frequency.h"

class AsyncCameraController {
 public:
  explicit AsyncCameraController(int buffer_size = 2);
  ~AsyncCameraController();

  CrossCamera& camera() { return camera_; }
  [[nodiscard]] const CrossCamera& camera() const { return camera_; }

  bool run();

  void stop();

  AsyncCameraController& operator>>(cv::Mat& frame);

  bool has_error() const {
    return error_;
  }

  int fps() const;

 private:
  void init();

  void RunAsync();

  void join();

  CrossCamera camera_;
  std::vector<cv::Mat> buffer_;
  int buffer_index_{0};

  std::thread thread_;
  bool stop_{false};
  bool terminate_{false};
  bool error_{false};

  mutable std::mutex mutex_;
  std::condition_variable cv_;

  Frequency<> freq_;
};

#endif // EMBED_CAMERA_ASYNC_CAMERA_CONTROLLER_H_
