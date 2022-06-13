//
// Created by YongGyu Lee on 2022/06/13.
//

#ifndef EMBED_DETECTOR_OBJECT_DETECTOR_H_
#define EMBED_DETECTOR_OBJECT_DETECTOR_H_

#include <cstdint>

class ObjectDetector {
 public:
  using milliseconds = int64_t;

 private:

  milliseconds last_detection_ = -100000;
  milliseconds run_model_override_t_ = 3000;
};

#endif // EMBED_DETECTOR_OBJECT_DETECTOR_H_
