//
// Created by YongGyu Lee on 2022/05/10.
//

#ifndef EMBED_CROSS_CAMERA_H_
#define EMBED_CROSS_CAMERA_H_

#include "opencv2/opencv.hpp"

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

#endif //EMBED_CROSS_CAMERA_H_
