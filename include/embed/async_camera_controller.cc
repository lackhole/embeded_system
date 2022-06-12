//
// Created by YongGyu Lee on 2022/05/10.
//

#include "embed/async_camera_controller.h"

AsyncCameraController::AsyncCameraController(int buffer_size)
  : buffer_(buffer_size)
{
  init();
}

AsyncCameraController::~AsyncCameraController() {
  join();
}

bool AsyncCameraController::run() {
  if (error_)
    return false;

  {
    std::lock_guard lck(mutex_);
    stop_ = false;
  }
  cv_.notify_all();
  return true;
}

void AsyncCameraController::stop() {
  {
    std::lock_guard lck(mutex_);
    stop_ = true;
  }
  cv_.notify_all();
}

AsyncCameraController& AsyncCameraController::operator>>(cv::Mat& frame) {
  std::lock_guard lck(mutex_);
  frame = buffer_[buffer_index_];
  return *this;
}

int AsyncCameraController::fps() const {
  std::lock_guard lck(mutex_);
  return freq_.freq();
}

void AsyncCameraController::init() {
  thread_ = std::thread(&AsyncCameraController::RunAsync, this);

  cv::Mat frame;
  camera_.set( cv::CAP_PROP_FORMAT, CV_8UC3);
//    camera_.set( cv::CAP_PROP_FRAME_WIDTH, 640 );
//    camera_.set( cv::CAP_PROP_FRAME_HEIGHT, 480 );
  camera_.set(cv::CAP_PROP_FPS, 60);
//    std::cout << "CAP_PROP_ZOOM: " << Camera.get(cv::CAP_PROP_ZOOM) << std::endl;
//    std::cout << "CAP_PROP_FOCUS: " << Camera.get(cv::CAP_PROP_FOCUS) << std::endl;

  if (!camera_.open()) {
    std::cerr << "Error opening the camera" << std::endl;
    error_ = true;
  }
}

void AsyncCameraController::RunAsync() {
  cv::Mat frame;
  std::unique_lock lck(mutex_);

  while (true) {
    cv_.wait(lck, [&]() {
      return !stop_ || terminate_;
    });

    if (terminate_)
      break;

    lck.unlock();

    camera_ >> frame;
    freq_.tick();

    lck.lock();
    buffer_[buffer_index_] = std::move(frame);
    buffer_index_ = (buffer_index_ + 1) % static_cast<int>(buffer_.size());
  }
}

void AsyncCameraController::join() {
  terminate_ = true;
  cv_.notify_all();
  if (thread_.joinable()) {
    thread_.join();
  }
}
