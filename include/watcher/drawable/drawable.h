//
// Created by YongGyu Lee on 2022/05/11.
//

#ifndef WATCHER_DRAWABLE_DRAWABLE_H_
#define WATCHER_DRAWABLE_DRAWABLE_H_

#include <type_traits>

#include "opencv2/opencv.hpp"

#include "watcher/drawable/rectangle.h"
#include "watcher/drawable/text.h"

namespace watcher {

template<typename T, typename = void>
struct is_drawable : std::false_type {};

template<typename T>
struct is_drawable<T, std::void_t<decltype(std::declval<const T&>().draw(std::declval<cv::Mat&>()))>> : std::true_type {};

template<typename T>
using is_drawable_t = typename is_drawable<T>::type;

template<typename T>
constexpr inline bool is_drawable_v = is_drawable<T>::value;

template<typename Drawable>
std::enable_if_t<is_drawable_v<Drawable>> draw(cv::Mat& image, const Drawable& d) {
  d.draw(image);
}

template<typename Drawable, typename... Drawables>
std::enable_if_t<std::conjunction_v<is_drawable<Drawable>, is_drawable<Drawables>...>>
draw(cv::Mat& image, const Drawable& d, const Drawables&... rest) {
  draw(image, d);
  draw(image, rest...);
}

} // namespace watcher

#endif // WATCHER_DRAWABLE_DRAWABLE_H_
