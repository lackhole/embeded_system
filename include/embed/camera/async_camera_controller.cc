//
// Created by YongGyu Lee on 2022/05/10.
//

#include "embed/camera/async_camera_controller.h"

void AsyncCameraController::open() {
  camera_.set( cv::CAP_PROP_FORMAT, CV_8UC3);
//    camera_.set( cv::CAP_PROP_FRAME_WIDTH, 640 );
//    camera_.set( cv::CAP_PROP_FRAME_HEIGHT, 480 );
  camera_.set(cv::CAP_PROP_FPS, 60);
//    std::cout << "CAP_PROP_ZOOM: " << Camera.get(cv::CAP_PROP_ZOOM) << std::endl;
//    std::cout << "CAP_PROP_FOCUS: " << Camera.get(cv::CAP_PROP_FOCUS) << std::endl;

  if (!camera_.open()) {
    std::cerr << "Error opening the camera\n";
  }
}

void AsyncCameraController::OnWakeUp() {
  camera_ >> frame_;
  freq_.tick();
  listener_(frame_);
}
