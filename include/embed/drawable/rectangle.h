//
// Created by YongGyu Lee on 2022/05/11.
//

#ifndef EMBED_DRAWABLE_RECTANGLE_H_
#define EMBED_DRAWABLE_RECTANGLE_H_

#include "opencv2/opencv.hpp"

#include "embed/drawable/macro.h"

class Rectangle {
 public:
  void draw(cv::Mat& image) const {
    cv::rectangle(image, tl(), br(), color(), thickness(), line_type(), shift());
  }

  EMBED_DRAWABLE_PROP(cv::Point, tl)
  EMBED_DRAWABLE_PROP(cv::Point, br)
  EMBED_DRAWABLE_PROP(cv::Scalar, color)
  EMBED_DRAWABLE_PROP(int, thickness, 1)
  EMBED_DRAWABLE_PROP(int, line_type, cv::LINE_8)
  EMBED_DRAWABLE_PROP(int, shift, 0)
};

#endif // EMBED_DRAWABLE_RECTANGLE_H_
