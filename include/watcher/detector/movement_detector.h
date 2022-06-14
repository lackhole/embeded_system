//
// Created by YongGyu Lee on 2022/06/13.
//

#ifndef WATCHER_DETECTOR_MOVEMENT_DETECTOR_H_
#define WATCHER_DETECTOR_MOVEMENT_DETECTOR_H_

#include <atomic>
#include <mutex>
#include <optional>
#include <unordered_set>
#include <vector>

#include "boost/signals2.hpp"
#include "opencv2/opencv.hpp"

#include "watcher/detector/object_detection_model.h"
#include "watcher/utility/async_runner.h"
#include "watcher/utility/ring_buffer.h"

namespace watcher {

class MovementDetector {
  using milliseconds = int64_t;

  struct Frame {
    milliseconds timestamp = -1;
    cv::Mat image;
  };

 public:
  using result_or_not = std::optional<ObjectDetectionModel::result_type>;

  MovementDetector();

  MovementDetector& LoadModelFromFile(const std::string& model_path, const std::string& labelmap_path);

  MovementDetector& LoadModelFromBuffer(const char* model, size_t model_size,
                                        const char* labelmap, size_t labelmap_size);

  MovementDetector& score_threshold(float threshold);
  float score_threshold() const { return score_threshold_; }

  MovementDetector& add_detection(std::string name);
  MovementDetector& remove_detection(const std::string& name);

  milliseconds inference_time() const { return inference_time_; }

  void feed(cv::Mat image, milliseconds timestamp);

  template<typename F>
  boost::signals2::connection add_listener(F func) {
    return listener_.connect(std::move(func));
  }


 private:
  void OnWakeUp();

  result_or_not invoke(const cv::Mat& image, milliseconds timestamp);

  bool movement_detected(const cv::Mat& image, milliseconds timestamp);

  void preprocess(const cv::Mat& src, cv::Mat& dst);

  static bool find_exceed(const cv::Mat& a, const cv::Mat& b, int threshold);

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

  boost::signals2::signal<void(const result_or_not&)> listener_;

  AsyncRunner async_runner_;

 public:
  cv::Mat temp_;
  RingBuffer<std::vector<cv::Rect>> diffs_{2};
//  RingBuffer<cv::Mat> diffs_{2};
};

} // namespace watcher

#endif // WATCHER_DETECTOR_MOVEMENT_DETECTOR_H_
