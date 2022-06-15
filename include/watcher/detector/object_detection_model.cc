//
// Created by YongGyu Lee on 2022/05/08.
//

#include "watcher/detector/object_detection_model.h"

#include <cstddef>
#include <fstream>
#include <map>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

#include "tensorflow/lite/c/c_api_types.h"
#include "tensorflow/lite/c/c_api.h"

#include "watcher/utility/logger.h"

namespace watcher {

template<typename T, typename = void>
struct is_reservable_impl : std::false_type {};

template<typename T>
struct is_reservable_impl<T, std::void_t<decltype(std::declval<T&>().reserve((size_t)0))>> : std::true_type {};

template<typename T>
struct is_reservable : is_reservable_impl<T> {};

template<typename T>
using is_reservable_t = typename is_reservable<T>::type;

template<typename T>
inline constexpr bool is_reservable_v = is_reservable<T>::value;

template<typename T, std::enable_if_t<is_reservable_v<T>, int> = 0>
static T make_reserved(size_t n) {
  T t;
  t.reserve(n);
  return t;
}

template<typename T, typename Allocator = std::allocator<T>>
static std::vector<T, Allocator> reserved_vector(size_t n) {
  return make_reserved<std::vector<T, Allocator>>(n);
}


/**
 * @brief nms2
 * Non maximum suppression with detection scores
 * @param srcRects
 * @param scores
 * @param result_idx
 * @param thresh
 * @param neighbors
 */
static std::vector<size_t> nms2(const std::vector<cv::Rect>& srcRects,
                                const std::vector<float>& scores,
                                float thresh,
                                int neighbors = 0,
                                float minScoresSum = 0.f)
{
  std::vector<size_t> result_idx;

  const size_t size = srcRects.size();
  if (!size)
    return {};

  assert(srcRects.size() == scores.size());

  // Sort the bounding boxes by the detection score
  std::multimap<float, size_t> idxs;
  for (size_t i = 0; i < size; ++i)
  {
    idxs.emplace(scores[i], i);
  }

  // keep looping while some indexes still remain in the indexes list
  while (idxs.size() > 0)
  {
    // grab the last rectangle
    auto lastElem = --std::end(idxs);
    const cv::Rect& rect1 = srcRects[lastElem->second];
    const auto index = lastElem->second;

    int neigborsCount = 0;
    float scoresSum = lastElem->first;

    idxs.erase(lastElem);

    for (auto pos = std::begin(idxs); pos != std::end(idxs); )
    {
      // grab the current rectangle
      const cv::Rect& rect2 = srcRects[pos->second];

      float intArea = static_cast<float>((rect1 & rect2).area());
      float unionArea = rect1.area() + rect2.area() - intArea;
      float overlap = intArea / unionArea;

      // if there is sufficient overlap, suppress the current bounding box
      if (overlap > thresh)
      {
        scoresSum += pos->first;
        pos = idxs.erase(pos);
        ++neigborsCount;
      }
      else
      {
        ++pos;
      }
    }
    if (neigborsCount >= neighbors && scoresSum >= minScoresSum)
      result_idx.push_back(index);
  }

  return result_idx;
}

ObjectDetectionModel::ObjectDetectionModel(std::string_view model_path, std::string_view labelmap_path) {
  load(model_path, labelmap_path);
}

void ObjectDetectionModel::loadFromBuffer(
  const char *model_buffer, size_t model_size,
  const char *labelmap_buffer, size_t labelmap_size)
{
  // Load model
  if (model_.isBuilt()) {
    return;
  }

  model_.loadBuffer(model_buffer, model_size);
  build();

  // Load labelmap
  labelmap_.clear();
  std::istringstream oss(std::string(labelmap_buffer, labelmap_size));

  std::string line;
  while (std::getline(oss, line)) {
    labelmap_.emplace_back(std::move(line));
  }
}

void ObjectDetectionModel::load(std::string_view model_path, std::string_view labelmap_path) {
  load_model(model_path);
  load_labelmap(labelmap_path);
}

void ObjectDetectionModel::load_model(std::string_view model_path) {
  if (model_.isBuilt()) {
    return;
  }

  model_.loadFile(model_path.data());
  build();
}

void ObjectDetectionModel::load_labelmap(std::string_view path) {
  std::ifstream ifs;

  if (ifs.open(path.data()); !ifs.is_open()) {
    Log.e("Failed to open ", path);
    std::terminate();
  }

  labelmap_.clear();
  std::string line;
  while (std::getline(ifs, line)) {
    labelmap_.emplace_back(std::move(line));
  }
}

void ObjectDetectionModel::build() {
  model_.setNumThreads(4);
  model_.build();

  if (const auto dim = model_.inputTensorDims(0); dim.size() >= 2) {
    input_size_ = cv::Size(dim[1], dim[2]);
  }

  Log.d(model_.summarize());
}

ObjectDetectionModel::result_type ObjectDetectionModel::invoke(const cv::Mat& image) {
  if (input_size_.empty()) {
    cv::resize(image, buffer_, {300, 300});
  } else {
    cv::resize(image, buffer_, input_size_);
  }

  cv::cvtColor(buffer_, buffer_, cv::COLOR_BGR2RGB);
  if (const auto t = model_.inputTensor(0); TfLiteTensorType(t) == kTfLiteFloat32) {
    buffer_.convertTo(buffer_, CV_32FC3, 1./255);
  }

  model_.setInput(buffer_.data);
  model_.invoke();

  result_type result;

  if (model_.outputTensorCount() == 4) {
    std::vector<float> rect_raw = model_.getOutput<float>(0);
    std::vector<float> class_raw = model_.getOutput<float>(1);
    std::vector<float> score_raw = model_.getOutput<float>(2);
    std::vector<float> num_detect_raw = model_.getOutput<float>(3);

    auto rect = make_reserved<std::vector<std::vector<float>>>(10);
    auto label = make_reserved<std::vector<std::string>>(10);
    const auto num_detect = static_cast<int>(num_detect_raw[0]);

    result.resize(num_detect);

    for (int i = 0; i < num_detect; ++i) {
      std::copy(rect_raw.data() + i * 4, rect_raw.data() + i * 4 + 4, result[i].rect);
      result[i].label = labelmap_[std::floor(class_raw[i] + 1.5)];
      result[i].score = score_raw[i];
    }
  } else {
    const auto rect_tensor = model_.outputTensor(0);
    const auto score_tensor = model_.outputTensor(1);
    const auto num_elem = TfLiteTensorDim(score_tensor, 1);
    const auto score_elem_size = TfLiteTensorDim(score_tensor, 2);
    const auto rect_elem_size = TfLiteTensorDim(rect_tensor, 2);

    std::vector<cv::Rect> rects;
    std::vector<float> scores;
    std::vector<size_t> classes;

    static constexpr auto min_score = 0.05;

    for (int i = 0; i < num_elem; ++i) {
      const auto first = static_cast<float*>(score_tensor->data.data) + i * score_elem_size;
      const auto last = static_cast<float*>(score_tensor->data.data) + (i + 1) * score_elem_size;
      const auto max_pos = std::max_element(first, last);

      if (*max_pos < min_score)
        continue;

      const auto rect = static_cast<float*>(rect_tensor->data.data) + rect_elem_size * i; // x, y, w, h

      const auto x = rect[0];
      const auto y = rect[1];
      const auto w = rect[2];
      const auto h = rect[3];

      rects.emplace_back((x - w/2), (y - h / 2), w, h);
      scores.emplace_back(*max_pos);
      classes.emplace_back(max_pos - first);
    }

    const auto idx = nms2(rects, scores, min_score);
    result.reserve(idx.size());

    for (const auto i : idx) {
      decltype(result)::value_type res;
      res.score = scores[i];
      res.label = labelmap_[classes[i]];
      res.rect[0] = static_cast<float>(rects[i].tl().y) / input_size_.height;
      res.rect[1] = static_cast<float>(rects[i].tl().x) / input_size_.width;
      res.rect[2] = static_cast<float>(rects[i].br().y) / input_size_.height;
      res.rect[3] = static_cast<float>(rects[i].br().x) / input_size_.width;
      result.emplace_back(std::move(res));
    }
  }

  return result;
}

} // namespace watcher
