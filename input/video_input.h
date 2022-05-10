//
// Created by YongGyu Lee on 2022/05/08.
//

#ifndef EMBED__VIDEO_INPUT_H_
#define EMBED__VIDEO_INPUT_H_

#include "i_image_generator.h"

#include "opencv2/opencv.hpp"

class VideoInput : public IImageGenerator {
 public:
  VideoInput(const std::string& path);

  void handle_key(int key) override;

  IImageGenerator& operator>>(cv::Mat& input) override;

 private:
  cv::VideoCapture video_;
  int skip_frame_ = 1;
  double fps_;
  double video_time_;
  int frame_idx = 0;
};

inline std::unique_ptr<IImageGenerator> make_generator(const std::string& path) {
  return std::make_unique<VideoInput>(path);
}

#endif //EMBED__VIDEO_INPUT_H_
