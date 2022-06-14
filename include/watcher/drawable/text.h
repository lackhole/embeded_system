//
// Created by YongGyu Lee on 2022/05/11.
//

#ifndef WATCHER_DRAWABLE_TEXT_H_
#define WATCHER_DRAWABLE_TEXT_H_

#include <string>

#include "opencv2/opencv.hpp"

#include "watcher/drawable/macro.h"

namespace watcher {

class Text {
 public:
  void draw(cv::Mat& image) const {
    cv::putText(image, text(), org(), font_face(), font_scale(), color(), thickness(), line_type(), bottom_left_origin());
  }

  WATCHER_DRAWABLE_PROP(std::string, text);
  WATCHER_DRAWABLE_PROP(cv::Point, org);
  WATCHER_DRAWABLE_PROP(int, font_face, cv::FONT_HERSHEY_DUPLEX);
  WATCHER_DRAWABLE_PROP(double, font_scale, 1);
  WATCHER_DRAWABLE_PROP(cv::Scalar, color);
  WATCHER_DRAWABLE_PROP(int, thickness, 1);
  WATCHER_DRAWABLE_PROP(int, line_type, cv::LINE_8);
  WATCHER_DRAWABLE_PROP(int, bottom_left_origin, false);
};

class BorderedText {
 public:
  void draw(cv::Mat& image) const {
    cv::putText(image, text(), org(), font_face(), font_scale(), color(), thickness(), line_type(), bottom_left_origin());
    cv::putText(image, text(), org(), font_face(), font_scale(), color_out(), thickness_out(), line_type(), bottom_left_origin());
  }

  WATCHER_DRAWABLE_PROP(std::string, text);
  WATCHER_DRAWABLE_PROP(cv::Point, org);
  WATCHER_DRAWABLE_PROP(int, font_face, cv::FONT_HERSHEY_DUPLEX);
  WATCHER_DRAWABLE_PROP(int, font_scale, 1);
  WATCHER_DRAWABLE_PROP(cv::Scalar, color);
  WATCHER_DRAWABLE_PROP(int, thickness, 1);
  WATCHER_DRAWABLE_PROP(int, line_type, cv::LINE_8);
  WATCHER_DRAWABLE_PROP(int, bottom_left_origin, false);
  WATCHER_DRAWABLE_PROP(int, thickness_out, 2);
  WATCHER_DRAWABLE_PROP(cv::Scalar, color_out, cv::Scalar(255,255,255));
};

} // namespace watcher

#endif // WATCHER_DRAWABLE_TEXT_H_
