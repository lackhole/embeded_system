//
// Created by YongGyu Lee on 2022/05/08.
//

#ifndef WATCHER_MOCK_INPUT_CAMERA_INPUT_H_
#define WATCHER_MOCK_INPUT_CAMERA_INPUT_H_

#include <memory>

#include "opencv2/opencv.hpp"

#include "watcher/mock_input/i_image_generator.h"

namespace watcher {

class CameraInput : public IImageGenerator {
 public:
  CameraInput(int index);

  IImageGenerator& operator>>(cv::Mat& input) override;

 private:
  cv::VideoCapture video_;
};

inline std::unique_ptr<IImageGenerator> make_generator(int index) {
  return std::make_unique<CameraInput>(index);
}

} // namespace watcher

#endif // WATCHER_MOCK_INPUT_CAMERA_INPUT_H_
