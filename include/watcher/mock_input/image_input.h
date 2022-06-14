//
// Created by YongGyu Lee on 2022/05/08.
//

#ifndef WATCHER_MOCK_INPUT_IMAGE_INPUT_H_
#define WATCHER_MOCK_INPUT_IMAGE_INPUT_H_

#include <memory>

#include "opencv2/opencv.hpp"

#include "watcher/mock_input/i_image_generator.h"

namespace watcher {

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

} // namespace watcher

#endif // WATCHER_MOCK_INPUT_IMAGE_INPUT_H_
