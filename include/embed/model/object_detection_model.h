//
// Created by YongGyu Lee on 2022/05/08.
//

#ifndef EMBED_MODEL_OBJECT_DETECTION_MODEL_H_
#define EMBED_MODEL_OBJECT_DETECTION_MODEL_H_

#include <vector>
#include <string>
#include <string_view>
#include <tuple>

#include "opencv2/opencv.hpp"
#include "cutemodel/cute_model.h"

#include "embed/model/async_model_runner.h"

class ObjectDetectionModel {
 public:
  using result_type = std::tuple<
    std::vector<std::vector<float>>, // rect
    std::vector<std::string>, // label
    std::vector<float>, // score
    int>; // num_detect

  ObjectDetectionModel() = default;
  ObjectDetectionModel(std::string_view model_path, std::string_view labelmap_path);

  void load(std::string_view model_path, std::string_view labelmap_path);

  result_type invoke(const cv::Mat& image);

 private:
  void load_model(std::string_view path);
  void load_labelmap(std::string_view path);

  cute::CuteModel model_;
  std::vector<std::string> labelmap_;
  cv::Mat buffer_;
};

using AsyncObjectDetector = AsyncModelRunner<ObjectDetectionModel, ObjectDetectionModel::result_type>;

#endif // EMBED_MODEL_OBJECT_DETECTION_MODEL_H_
