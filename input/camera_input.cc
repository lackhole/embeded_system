//
// Created by YongGyu Lee on 2022/05/08.
//

#include "camera_input.h"

CameraInput::CameraInput(int index) {
  video_.open(index);
}

IImageGenerator& CameraInput::operator>>(cv::Mat& input) {
  video_ >> input;
  return *this;
}
