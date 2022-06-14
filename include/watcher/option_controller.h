//
// Created by YongGyu Lee on 2022/06/12.
//

#ifndef WATCHER_OPTION_H_
#define WATCHER_OPTION_H_

#include <string>
#include <unordered_set>

#include "watcher/network/protocol.h"

namespace watcher {

class OptionController {
 public:
  static OptionController& get() {
    static auto inst = new OptionController();
    return *inst;
  }

  int criteria() const noexcept { return criteria_; }


  void fetch() {
//    protocol_.Get
  }

 private:
  OptionController() = default;

  float criteria_ = 0.5;
  std::unordered_set<std::string> detect_obj_;
  int max_fps_ = 60;

  std::string model_path_;
  std::string labelmap_path_;

  Protocol protocol_;
};

} // namespace watcher

#endif // WATCHER_OPTION_H_
