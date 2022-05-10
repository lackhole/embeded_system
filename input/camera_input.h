//
// Created by YongGyu Lee on 2022/05/08.
//

#ifndef EMBED__CAMERA_INPUT_H_
#define EMBED__CAMERA_INPUT_H_

#include "input/i_image_generator.h"

#include "opencv2/opencv.hpp"

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

#endif //EMBED__CAMERA_INPUT_H_
