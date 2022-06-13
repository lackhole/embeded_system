//
// Created by YongGyu Lee on 2022/05/10.
//

#ifndef EMBED_CAMERA_ASYNC_CAMERA_CONTROLLER_H_
#define EMBED_CAMERA_ASYNC_CAMERA_CONTROLLER_H_

#include <vector>

#include "boost/signals2.hpp"
#include "opencv2/opencv.hpp"

#include "embed/camera/cross_camera.h"
#include "embed/utility/async_runner.h"
#include "embed/utility/frequency.h"
#include "embed/utility/ring_buffer.h"

class AsyncCameraController {
 public:
  explicit AsyncCameraController() : async_runner_(true) {
    async_runner_.AddWakeUpListener([this](){ OnWakeUp(); });
  }

  void open();

  bool is_open() const { return camera_.is_opened(); }

  CrossCamera& camera() { return camera_; }
  [[nodiscard]] const CrossCamera& camera() const { return camera_; }

  int fps() const { return freq_.freq(); }

  template<typename F>
  boost::signals2::connection add_listener(F func) {
    return listener_.connect(std::move(func));
  }

  void run() {
    async_runner_.run();
  }

 private:
  void OnWakeUp();

  CrossCamera camera_;
  cv::Mat frame_;

  Frequency<> freq_;
  boost::signals2::signal<void(cv::Mat)> listener_;

  AsyncRunner async_runner_;
};


#endif // EMBED_CAMERA_ASYNC_CAMERA_CONTROLLER_H_
