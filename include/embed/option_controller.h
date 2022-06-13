//
// Created by YongGyu Lee on 2022/06/12.
//

#ifndef EMBED_OPTION_H_
#define EMBED_OPTION_H_

#include <string>
#include <unordered_set>

#include "embed/network/protocol.h"

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

  int criteria_ = 50;
  std::unordered_set<std::string> detect_obj_;
  int max_fps_ = 60;

  std::string model_path_;
  std::string labelmap_path_;

  Protocol protocol_;
};

#endif // EMBED_OPTION_H_
