//
// Created by YongGyu Lee on 2022/05/08.
//

#include "watcher/mock_input/camera_input.h"

#include "opencv2/opencv.hpp"

namespace watcher {

CameraInput::CameraInput(int index) {
  video_.open(index);
}

IImageGenerator& CameraInput::operator>>(cv::Mat& input) {
  video_ >> input;
  return *this;
}

} // namespace watcher
