//
// Created by YongGyu Lee on 2022/05/08.
//

#include "watcher/mock_input/image_input.h"

#include "opencv2/opencv.hpp"

namespace watcher {

ImageInput::ImageInput(const std::string& path, int flag) {
  image_ = cv::imread(path, flag);
}

ImageInput::ImageInput(const cv::Mat& image) {
  image_ = image.clone();
}

IImageGenerator& ImageInput::operator>>(cv::Mat& input) {
  input = image_.clone();
  return *this;
}

} // namespace watcher
