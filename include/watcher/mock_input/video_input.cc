//
// Created by YongGyu Lee on 2022/05/08.
//

#include "watcher/mock_input/video_input.h"

#include <algorithm>
#include <string>

namespace watcher {

VideoInput::VideoInput(const std::string& path) {
  video_.open(path);
  fps_ = video_.get(cv::CAP_PROP_FPS);
  video_time_ = video_.get(cv::CAP_PROP_POS_MSEC);
}

void VideoInput::handle_key(int key) {
  switch(key) {
    case kLEFT: // LEFT
      skip_frame_ = -fps_ * 5;
      break;

    case kRIGHT: // RIGHT
      skip_frame_ = fps_ * 5;
      break;
  }
}

IImageGenerator& VideoInput::operator>>(cv::Mat& input) {
  if (skip_frame_ > 0) {
    for (int i = 0; i < skip_frame_; ++i) {
      video_ >> input;
      if (input.empty()) {
        break;
      }
      ++frame_idx;
    }
  } else {
    frame_idx = std::max(0, frame_idx - skip_frame_);
    video_.set(cv::CAP_PROP_POS_MSEC, frame_idx * 1./fps_);
  }
  skip_frame_ = 1;
  return *this;
}

} // namespace watcher
