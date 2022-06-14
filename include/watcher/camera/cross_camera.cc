//
// Created by YongGyu Lee on 2022/05/10.
//

#include "watcher/camera/cross_camera.h"

#ifdef __linux__ // Raspberry PI

#include "raspicam/raspicam_cv.h"

namespace watcher {

class CrossCamera::Impl {
 public:
  bool open() { return video_.open(); }
  bool is_opened() const { return video_.isOpened(); }
  void release() { video_.release(); }

  double get(cv::VideoCaptureProperties pid) { return video_.get(pid); }

  bool set(cv::VideoCaptureProperties pid, double value) { return video_.set(pid, value); }

  void operator>>(cv::Mat& input) { video_.grab(), video_.retrieve(input); }
 private:
  raspicam::RaspiCam_Cv video_;
};

} // namespace watcher

#else // PC

#include "opencv2/opencv.hpp"

namespace watcher {

class CrossCamera::Impl {
 public:
  bool open() { return video_.open(0); }
  bool is_opened() const { return video_.isOpened(); }
  void release() { video_.release(); }

  double get(cv::VideoCaptureProperties pid) const { return video_.get(pid); }

  bool set(cv::VideoCaptureProperties pid, double value) { return video_.set(pid, value); }

  void operator>>(cv::Mat& input) {
    video_ >> input;
  }
 private:
  cv::VideoCapture video_;
};

#endif

CrossCamera::CrossCamera() : pimpl_(nullptr) {
  pimpl_ = new CrossCamera::Impl();
}

CrossCamera::~CrossCamera() {
  delete pimpl_;
}

bool CrossCamera::open() {
  return pimpl_->open();
}

bool CrossCamera::is_opened() const {
  return pimpl_->is_opened();
}

void CrossCamera::release() {
  return pimpl_->release();
}

double CrossCamera::get(cv::VideoCaptureProperties pid) const {
  return pimpl_->get(pid);
}

bool CrossCamera::set(cv::VideoCaptureProperties pid, double value) {
  return pimpl_->set(pid, value);
}

CrossCamera& CrossCamera::operator>>(cv::Mat& input) {
  (*pimpl_) >> input;
  return *this;
}

} // namespace watcher
