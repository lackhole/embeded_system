//
// Created by YongGyu Lee on 2022/05/10.
//

#ifndef WATCHER_CAMERA_CROSS_CAMERA_H_
#define WATCHER_CAMERA_CROSS_CAMERA_H_

#include "opencv2/opencv.hpp"

namespace watcher {

class CrossCamera {
 public:
  CrossCamera();
  ~CrossCamera();

  bool open();
  [[nodiscard]] bool is_opened() const;
  void release();

  [[nodiscard]] double get(cv::VideoCaptureProperties pid) const;

  bool set(cv::VideoCaptureProperties pid, double value);

  CrossCamera& operator>>(cv::Mat& input);

 private:
  class Impl;
  class Impl* pimpl_;
};

} // namespace watcher

#endif // WATCHER_CAMERA_CROSS_CAMERA_H_
