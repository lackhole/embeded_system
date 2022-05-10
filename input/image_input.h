//
// Created by YongGyu Lee on 2022/05/08.
//

#ifndef EMBED__IMAGE_INPUT_H_
#define EMBED__IMAGE_INPUT_H_

#include "input/i_image_generator.h"

#include "opencv2/opencv.hpp"

class ImageInput : public IImageGenerator {
 public:
  ImageInput(const std::string& path, int flag);
  ImageInput(const cv::Mat& image);

  IImageGenerator& operator>>(cv::Mat& input) override;

 private:
  cv::Mat image_;
};

inline std::unique_ptr<IImageGenerator> make_generator(const cv::Mat& image) {
  return std::make_unique<ImageInput>(image);
}

#endif //EMBED__IMAGE_INPUT_H_
