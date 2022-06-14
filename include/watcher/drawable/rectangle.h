//
// Created by YongGyu Lee on 2022/05/11.
//

#ifndef WATCHER_DRAWABLE_RECTANGLE_H_
#define WATCHER_DRAWABLE_RECTANGLE_H_

#include "opencv2/opencv.hpp"

#include "watcher/drawable/macro.h"

namespace watcher {

class Rectangle {
 public:
  void draw(cv::Mat& image) const {
    cv::rectangle(image, tl(), br(), color(), thickness(), line_type(), shift());
  }

  WATCHER_DRAWABLE_PROP(cv::Point, tl)
  WATCHER_DRAWABLE_PROP(cv::Point, br)
  WATCHER_DRAWABLE_PROP(cv::Scalar, color)
  WATCHER_DRAWABLE_PROP(int, thickness, 1)
  WATCHER_DRAWABLE_PROP(int, line_type, cv::LINE_8)
  WATCHER_DRAWABLE_PROP(int, shift, 0)
};

} // namespace watcher

#endif // WATCHER_DRAWABLE_RECTANGLE_H_
