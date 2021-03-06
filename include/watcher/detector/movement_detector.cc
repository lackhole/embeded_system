//
// Created by YongGyu Lee on 2022/06/13.
//

#include "watcher/detector/movement_detector.h"

#include <cmath>
#include <string>
#include <vector>

#include "opencv2/opencv.hpp"

#include "watcher/utility/date_time.h"
#include "watcher/utility/logger.h"

namespace watcher {

MovementDetector::MovementDetector() {
  async_runner_.AddWakeUpListener([this](){ OnWakeUp(); });
}

MovementDetector& MovementDetector::LoadModelFromFile(const std::string& model_path, const std::string& labelmap_path) {
  model_.load(model_path, labelmap_path);
  return *this;
}

MovementDetector& MovementDetector::LoadModelFromBuffer(const char* model, size_t model_size,
                                                        const char* labelmap, size_t labelmap_size) {
  model_.loadFromBuffer(model, model_size, labelmap, labelmap_size);
  return *this;
}

MovementDetector& MovementDetector::score_threshold(float threshold) {
  score_threshold_ = threshold;
  return *this;
}

MovementDetector& MovementDetector::add_detection(std::string name) {
  std::lock_guard lck(m_);
  desired_object_.emplace(std::move(name));
  return *this;
}
MovementDetector& MovementDetector::remove_detection(const std::string& name) {
  std::lock_guard lck(m_);
  desired_object_.erase(name);
  return *this;
}

void MovementDetector::feed(cv::Mat image, milliseconds timestamp) {
  input_.store(Frame{timestamp, image});
  async_runner_.run();
}

void MovementDetector::OnWakeUp() {
  if(const auto data = input_.load(); data) {
    const auto invoke_result = invoke(data->image, data->timestamp);
    listener_(invoke_result);
  }
}

MovementDetector::result_or_not MovementDetector::invoke(const cv::Mat& image, milliseconds timestamp) {
  const auto t0 = DateTime<>::now().milliseconds();
  const auto mvd = movement_detected(image, timestamp);
  object_detected_ = false;

  if (!mvd) {
    inference_time_ = static_cast<int>(DateTime<>::now().milliseconds() - t0);
    return std::nullopt;
  }

  const auto detection_result = model_.invoke(image);
  ObjectDetectionModel::result_type out_result;

  {
    std::lock_guard lck(m_);
    for (const auto& detection : detection_result) {
//      if (auto it = desired_object_.find(detection.label);
//        it != desired_object_.end() && detection.score >= score_threshold_) {
      //   out_result.emplace_back(detection);
      if (detection.score >= score_threshold_) {
        out_result.emplace_back(detection);
      }
//      }
    }
  }
  last_detection_ = timestamp;

  if (criteria_.timestamp + run_model_override_t_ < timestamp) {
    preprocess(image, criteria_.image);
    criteria_.timestamp = timestamp;
  }
  inference_time_ = static_cast<int>(DateTime<>::now().milliseconds() - t0);

  if (out_result.empty()) {
    return std::nullopt;
  }

  object_detected_ = true;
  return out_result;
}

bool MovementDetector::movement_detected(const cv::Mat& image, milliseconds timestamp) {

  if (criteria_.image.empty()) {
    preprocess(image, criteria_.image);
    criteria_.timestamp = timestamp;
    return true;
  }

  preprocess(image, current_.image);
  current_.timestamp = timestamp;

  const auto m = cv::mean(image);
  const auto avg = (m[0]+m[1]+m[2])/3;

  cv::absdiff(criteria_.image, current_.image, temp_);
  cv::dilate(temp_, temp_, cv::getStructuringElement(cv::MORPH_RECT, {10, 10}));
  cv::threshold(temp_, temp_, avg * 0.38, 255, cv::THRESH_BINARY);

  std::vector<std::vector<cv::Point>> contours;
  cv::findContours(temp_, contours, cv::RETR_LIST, cv::CHAIN_APPROX_NONE);

  std::vector<cv::Rect> movement_area;
  movement_area.reserve(contours.size());
  for (const auto& contour : contours) {
    if (cv::contourArea(contour) < 50) {
      continue;
    }

    movement_area.emplace_back(cv::boundingRect(contour));
  }
  bbox_(movement_area);
//  diffs_.store(std::move(movement_area));
//  diffs_.store(temp_);
  if (object_detected_ || timestamp > last_detection_ + run_model_override_t_) {
    return true;
  }

  return find_exceed(criteria_.image, current_.image, diff_threshold_);
}

void MovementDetector::preprocess(const cv::Mat& src, cv::Mat& dst) {
  cv::cvtColor(src, dst, cv::COLOR_BGR2GRAY);
  cv::GaussianBlur(dst, dst, blur_size_, 0);
}

bool MovementDetector::find_exceed(const cv::Mat& a, const cv::Mat& b, int threshold) {
  const auto size = a.cols * a.rows * a.channels();
  for (int i = size - 1; i >= 0; --i) {
    if (std::abs(a.data[i] - b.data[i]) > threshold)
      return true;
  }
  return false;
}

} // namespace watcher
