//
// Created by YongGyu Lee on 2022/05/08.
//

#ifndef WATCHER_MODEL_OBJECT_DETECTION_MODEL_H_
#define WATCHER_MODEL_OBJECT_DETECTION_MODEL_H_

#include <string>
#include <string_view>
#include <vector>

#include "opencv2/opencv.hpp"
#include "cutemodel/cute_model.h"

namespace watcher {

class ObjectDetectionModel {
 public:
  struct Detection {
    float rect[4];
    std::string label;
    float score;
  };
  using result_type = std::vector<Detection>;

  ObjectDetectionModel() = default;
  ObjectDetectionModel(std::string_view model_path, std::string_view labelmap_path);

  void load(std::string_view model_path, std::string_view labelmap_path);

  void loadFromBuffer(const char* model_buffer, size_t model_size,
                      const char* labelmap_buffer, size_t labelmap_size);

  result_type invoke(const cv::Mat& image);

  const cv::Size& input_size() const;

 private:
  void load_model(std::string_view path);
  void load_labelmap(std::string_view path);

  void build();

  cute::CuteModel model_;
  std::vector<std::string> labelmap_;
  cv::Mat buffer_;
  cv::Size input_size_;
};

} // namespace watcher

#endif // WATCHER_MODEL_OBJECT_DETECTION_MODEL_H_
