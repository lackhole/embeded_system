//
// Created by YongGyu Lee on 2022/06/13.
//

#ifndef EMBED_DETECTOR_MOVEMENT_DETECTOR_H_
#define EMBED_DETECTOR_MOVEMENT_DETECTOR_H_

#include <cmath>
#include <vector>

#include "opencv2/opencv.hpp"

#include "embed/detector/object_detection_model.h"
#include "embed/utility/date_time.h"

class MovementDetector {
  using milliseconds = int64_t;

  struct Frame {
    milliseconds timestamp = -1;
    cv::Mat image;
  };

 public:
  MovementDetector() = default;

  bool invoke(const cv::Mat& image, milliseconds timestamp) {
    bool run_model = false;

    if (timestamp > last_detection_ + run_model_override_t_) {
      return true;
    }

    if (criteria_.image.empty()) {
      preprocess(image, criteria_.image);
      criteria_.timestamp = timestamp;
      return true;
    }

    preprocess(image, current_.image);
    current_.timestamp = timestamp;

//    return find_exceed(criteria_.image, current_.image, threshold_))
  }

 private:
  void preprocess(const cv::Mat& src, cv::Mat& dst) {
    cv::cvtColor(src, dst, cv::COLOR_BGR2GRAY);
    cv::GaussianBlur(dst, dst, blur_size_, 0);
  }

  static bool find_exceed(const cv::Mat& a, const cv::Mat& b, int threshold) {
    const auto size = a.cols * a.rows * a.channels();
    for (int i = size - 1; i >= 0; --i) {
      if (std::abs(a.data[i] - b.data[i]) > threshold)
        return true;
    }
    return false;
  }

  milliseconds last_detection_ = -100000;
  milliseconds run_model_override_t_ = 3000;

  Frame criteria_;
  Frame current_;
  cv::Size blur_size_{5, 5};
  int threshold_ = 20;

  RingBuffer<ObjectDetectionModel::result_type> output_{2};
};

#endif // EMBED_DETECTOR_MOVEMENT_DETECTOR_H_
