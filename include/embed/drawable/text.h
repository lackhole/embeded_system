//
// Created by YongGyu Lee on 2022/05/11.
//

#ifndef EMBED_DRAWABLE_TEXT_H_
#define EMBED_DRAWABLE_TEXT_H_

#include <string>

#include "opencv2/opencv.hpp"

#include "embed/drawable/macro.h"

class Text {
 public:
  void draw(cv::Mat& image) const {
    cv::putText(image, text(), org(), font_face(), font_scale(), color(), thickness(), line_type(), bottom_left_origin());
  }

  EMBED_DRAWABLE_PROP(std::string, text);
  EMBED_DRAWABLE_PROP(cv::Point, org);
  EMBED_DRAWABLE_PROP(int, font_face, cv::FONT_HERSHEY_DUPLEX);
  EMBED_DRAWABLE_PROP(int, font_scale, 1);
  EMBED_DRAWABLE_PROP(cv::Scalar, color);
  EMBED_DRAWABLE_PROP(int, thickness, 1);
  EMBED_DRAWABLE_PROP(int, line_type, cv::LINE_8);
  EMBED_DRAWABLE_PROP(int, bottom_left_origin, false);
};

#endif // EMBED_DRAWABLE_TEXT_H_
