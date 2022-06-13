//
// Created by YongGyu Lee on 2022/05/30.
//

#ifndef EMBED_MODEL_ASYNC_MODEL_RUNNER_H_
#define EMBED_MODEL_ASYNC_MODEL_RUNNER_H_

#include <condition_variable>
#include <mutex>
#include <optional>
#include <thread>
#include <utility>
#include <vector>

#include "opencv2/opencv.hpp"

#include "embed/utility/async_runner.h"
#include "embed/utility/ring_buffer.h"
#include "embed/utility/frequency.h"

template<typename Model, typename Out>
class AsyncModelRunner {
 public:
  using output_type = Out;
  using model_type = Model;

  explicit AsyncModelRunner(int buffer_size = 2)
    : input_(buffer_size), output_(buffer_size)
  {
    async_runner_.AddWakeUpListener([this](){ OnWakeUp(); });
  }

  model_type& model() noexcept { return model_; }
  const model_type& model() const noexcept { return model_; }

  void feed(cv::Mat image) {
    input_.store(std::move(image));
    async_runner_.run();
  }

  std::optional<output_type> retrieve() {
    return output_.load();
  }

  int fps() const {
    std::lock_guard lck(m_);
    return freq_.freq();
  }

 private:
  void OnWakeUp() {
    const auto input = input_.load();
    if (input) {
      output_.store(model_.invoke(*input));
      freq_.tick();
    }
  }

  RingBuffer<cv::Mat> input_;
  model_type model_;
  RingBuffer<output_type> output_;

  Frequency<> freq_;
  mutable std::mutex m_;

  AsyncRunner async_runner_;
};

#endif // EMBED_MODEL_ASYNC_MODEL_RUNNER_H_
