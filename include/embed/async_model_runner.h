//
// Created by YongGyu Lee on 2022/05/30.
//

#ifndef EMBED_ASYNC_MODEL_RUNNER_H_
#define EMBED_ASYNC_MODEL_RUNNER_H_

#include <condition_variable>
#include <mutex>
#include <optional>
#include <thread>
#include <utility>
#include <vector>

#include "opencv2/opencv.hpp"

#include "embed/ring_buffer.h"
#include "embed/frequency.h"

template<typename Model, typename Out>
class AsyncModelRunner {
 public:
  using output_type = Out;
  using model_type = Model;

  explicit AsyncModelRunner(const std::string& model_path,
                            const std::string& labelmap_path,
                            int buffer_size = 2)
    : input_(buffer_size), output_(buffer_size)
  {
    model_.load(model_path, labelmap_path);
    thread_ = std::thread(&AsyncModelRunner<model_type, output_type>::RunAsync, this);
  }

  ~AsyncModelRunner() { join(); }

  void feed(cv::Mat image) {
    input_.store(std::move(image));
    run();
  }

  std::optional<output_type> retrieve() {
    return output_.load();
  }

  void run() {
    {
      std::lock_guard lck(mutex_);
      stop_ = false;
    }
    cv_.notify_all();
  }

  void stop() {
    {
      std::lock_guard lck(mutex_);
      stop_ = true;
    }
    cv_.notify_all();
  }

  int fps() const {
    return freq_.freq();
  }

 private:
  void RunAsync() {
    output_type out;
    std::unique_lock lck(mutex_);

    while(true) {
      cv_.wait(lck, [&]() {
        return !stop_ || terminate_;
      });

      if (terminate_)
        break;

      lck.unlock();
      const auto input = input_.load();
      if (input) {
        out = model_.invoke(*input);
        output_.store(out);
        freq_.tick();
      }
      lck.lock();
      stop_ = true;
    }
  }

  void join() {
    terminate_ = true;
    cv_.notify_all();
    if (thread_.joinable()) {
      thread_.join();
    }
  }

  RingBuffer<cv::Mat> input_;
  model_type model_;
  RingBuffer<output_type> output_;

  std::thread thread_;
  bool stop_{false};
  bool terminate_{false};

  mutable std::mutex mutex_;
  std::condition_variable cv_;

  Frequency<> freq_;
};

#endif // EMBED_ASYNC_MODEL_RUNNER_H_
