//
// Created by YongGyu Lee on 2022/05/08.
//

#include "embed/detector/object_detection_model.h"

#include <cstddef>
#include <memory>
#include <fstream>
#include <type_traits>
#include <vector>

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
T make_reserved(size_t n) {
  T t;
  t.reserve(n);
  return t;
}

template<typename T, typename Allocator = std::allocator<T>>
std::vector<T, Allocator> reserved_vector(size_t n) {
  return make_reserved<std::vector<T, Allocator>>(n);
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
  model_.setNumThreads(4);
  model_.build();

  std::cout << model_.summarize() << std::endl;

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
  model_.setNumThreads(4);
  model_.build();

  std::cout << model_.summarize() << std::endl;
}

void ObjectDetectionModel::load_labelmap(std::string_view path) {
  std::ifstream ifs;

  if (ifs.open(path.data()); !ifs.is_open()) {
    std::cerr << "Failed to open" << path << std::endl;
    std::terminate();
  }

  labelmap_.clear();
  std::string line;
  while (std::getline(ifs, line)) {
    labelmap_.emplace_back(std::move(line));
  }
}

template<typename T>
static void print_container(const T& t) {
  for (const auto& elem : t) {
    std::cout << elem << " ";
  }
  std::cout << '\n';
}

ObjectDetectionModel::result_type ObjectDetectionModel::invoke(const cv::Mat& image) {
  cv::resize(image, buffer_, {300, 300});
  model_.setInput(buffer_.data);
  model_.invoke();

  std::vector<float> rect_raw = model_.getOutput<float>(0);
  std::vector<float> class_raw = model_.getOutput<float>(1);
  std::vector<float> score_raw = model_.getOutput<float>(2);
  std::vector<float> num_detect_raw = model_.getOutput<float>(3);

//  std::cout << "==========\n";
//  print_container(rect_raw);
//  print_container(class_raw);
//  print_container(score_raw);
//  print_container(num_detect_raw);


  auto rect = make_reserved<std::vector<std::vector<float>>>(10);
  auto label = make_reserved<std::vector<std::string>>(10);
  const auto num_detect = static_cast<int>(num_detect_raw[0]);

  result_type result(num_detect);

  for (int i = 0; i < num_detect; ++i) {
    std::copy(rect_raw.data() + i * 4, rect_raw.data() + i * 4 + 4, result[i].rect);
    result[i].label = labelmap_[std::floor(class_raw[i] + 1.5)];
    result[i].score = score_raw[i];
  }

  return result;
}
