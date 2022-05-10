//
// Created by YongGyu Lee on 2022/05/08.
//

#ifndef EMBED_I_IMAGE_GENERATOR_H_
#define EMBED_I_IMAGE_GENERATOR_H_

#include "opencv2/opencv.hpp"

#include <memory>

enum KeyBoard {
  kLEFT = 2,
  kRIGHT = 3,
  kTAB = 9,
  kESC = 27,
  kSPACE = 32,
};

class IImageGenerator {
 public:
  IImageGenerator() = default;
  virtual ~IImageGenerator() = default;

  virtual void handle_key(int key) {};

  virtual IImageGenerator& operator>>(cv::Mat& input) = 0;
};


#endif // EMBED_I_IMAGE_GENERATOR_H_
