//
// Created by YongGyu Lee on 2022/06/13.
//

#ifndef EMBED_DETECTOR_MOVEMENT_DETECTOR_H_
#define EMBED_DETECTOR_MOVEMENT_DETECTOR_H_

#include <atomic>
#include <cmath>
#include <mutex>
#include <optional>
#include <unordered_set>
#include <vector>

#include "opencv2/opencv.hpp"

#include "embed/detector/object_detection_model.h"
#include "embed/utility/async_runner.h"
#include "embed/utility/date_time.h"
#include "embed/utility/logger.h"

class MovementDetector {
  using milliseconds = int64_t;

  struct Frame {
    milliseconds timestamp = -1;
    cv::Mat image;
  };

 public:
  MovementDetector() {
    async_runner_.AddWakeUpListener([this](){ OnWakeUp(); });
  }

  MovementDetector& LoadModelFromFile(const std::string& model_path, const std::string& labelmap_path) {
    model_.load(model_path, labelmap_path);
    return *this;
  }

  MovementDetector& LoadModelFromBuffer(const char* model, size_t model_size,
                                        const char* labelmap, size_t labelmap_size) {
    model_.loadFromBuffer(model, model_size, labelmap, labelmap_size);
    return *this;
  }

  MovementDetector& score_threshold(float threshold) {
    score_threshold_ = threshold;
    return *this;
  }

  MovementDetector& add_detection(std::string name) {
    std::lock_guard lck(m_);
    desired_object_.emplace(std::move(name));
    return *this;
  }
  MovementDetector& remove_detection(const std::string& name) {
    std::lock_guard lck(m_);
    desired_object_.erase(name);
    return *this;
  }

  void feed(cv::Mat image, milliseconds timestamp) {
    input_.store(Frame{timestamp, image});
    async_runner_.run();
  }

  std::optional<ObjectDetectionModel::result_type> retrieve() {
    return output_.load();
  }

  milliseconds inference_time() const { return inference_time_; }

 private:
  void OnWakeUp() {
    if(const auto data = input_.load(); data) {
      const auto invoke_result = invoke(data->image, data->timestamp);
      output_.store(invoke_result);
    }
  }

  std::optional<ObjectDetectionModel::result_type> invoke(const cv::Mat& image, milliseconds timestamp) {
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
        if (auto it = desired_object_.find(detection.label);
          it != desired_object_.end() && detection.score >= score_threshold_) {
          out_result.emplace_back(detection);
        }
      }
    }
    last_detection_ = timestamp;

    if (out_result.empty()) {
      preprocess(image, criteria_.image);
      criteria_.timestamp = timestamp;
      inference_time_ = static_cast<int>(DateTime<>::now().milliseconds() - t0);
      return std::nullopt;
    }

    object_detected_ = true;

    inference_time_ = static_cast<int>(DateTime<>::now().milliseconds() - t0);

    return out_result;
  }

  bool movement_detected(const cv::Mat& image, milliseconds timestamp) {
    if (object_detected_ || timestamp > last_detection_ + run_model_override_t_) {
      return true;
    }

    if (criteria_.image.empty()) {
      preprocess(image, criteria_.image);
      criteria_.timestamp = timestamp;
      return true;
    }

    preprocess(image, current_.image);
    current_.timestamp = timestamp;

    return find_exceed(criteria_.image, current_.image, diff_threshold_);
  }

  void preprocess(const cv::Mat& src, cv::Mat& dst) {
    cv::cvtColor(src, dst, cv::COLOR_BGR2GRAY);
    cv::GaussianBlur(dst, dst, blur_size_, 0);
  }

  static bool find_exceed(const cv::Mat& a, const cv::Mat& b, int threshold) {
    const auto size = a.cols * a.rows * a.channels();
    for (int i = size - 1; i >= 0; --i) {
      if (std::abs(a.data[i] - b.data[i]) > threshold)
        return true;
    }
    return false;
  }

  AsyncRunner async_runner_;

  RingBuffer<Frame> input_{2};

  mutable std::mutex m_;

  bool object_detected_ = false;
  milliseconds last_detection_ = -100000;
  milliseconds run_model_override_t_ = 3000;

  Frame criteria_;
  Frame current_;
  cv::Size blur_size_{5, 5};
  int diff_threshold_ = 40;

  ObjectDetectionModel model_;
  std::atomic<int> inference_time_{-1};
  float score_threshold_ = 0.5;
  std::unordered_set<std::string> desired_object_{"person", "dog", "cat"};
  RingBuffer<ObjectDetectionModel::result_type> output_{2};
};

#endif // EMBED_DETECTOR_MOVEMENT_DETECTOR_H_
