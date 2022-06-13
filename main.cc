#include <atomic>
#include <iostream>
#include <string>
#include <memory>
#include <vector>

#include "opencv2/opencv.hpp"

#include "embed/camera/async_camera_controller.h"
#include "embed/detector/movement_detector.h"
#include "embed/detector/object_detection_model.h"
#include "embed/network/async_video_client.h"
#include "embed/utility/date_time.h"
#include "embed/option_controller.h"

#include "embed/drawable/drawable.h"

#if __linux__
constexpr auto kPWD = "/home/pi/embeded_system";
#else
constexpr auto kPWD = "/Users/yonggyulee/CLionProjects/embed";
#endif

enum Key {
  kLEFT = 2,
  kRIGHT = 3,
  kTAB = 9,
  kESC = 27,
  kSPACE = 32,
};

std::optional<std::pair<std::string, std::string>> load_model_data(std::string url, std::string port) {
  std::string model_buffer;
  std::string labelmap_buffer;

  TcpClient client(url, port);
  Protocol protocol;
  auto response = protocol.Get(
    client,
    "model/ssd_mobilenet_v1_1_metadata_1.tflite"
  );

  auto it = response.find("data");
  if (it == response.end()) {
    Log.d("Failed to load model");
    return std::nullopt;
  }

  model_buffer = std::move(it->second);

  response = protocol.Get(
    client,
    "model/labelmap.txt"
  );

  it = response.find("data");
  if (it == response.end()) {
    Log.d("Failed to load labelmap");
    return std::nullopt;
  }

  labelmap_buffer = std::move(it->second);

  return std::make_pair(std::move(model_buffer), std::move(labelmap_buffer));
}

int main(int argc, char* argv[]) {
  std::string url;
  std::string port;

  if (argc != 3) {
    std::cerr << "Usage: [URL] [PORT]" << std::endl;
    return EXIT_FAILURE;
  }

  url = argv[1];
  port = argv[2];

  cv::Mat view;
  RingBuffer<cv::Mat> frames;
  std::atomic<bool> updated{false};
  cv::Mat frame;
  cv::Mat frame_prev;
  AsyncCameraController camera;
  camera.open();
  if (!camera.is_open()) {
    return EXIT_FAILURE;
  }

  const double scale = 1;

  const auto model_data = load_model_data(url, port);
  if (!model_data)
    return EXIT_FAILURE;

  MovementDetector detector;

  detector.LoadModelFromBuffer(model_data->first.data(), model_data->first.size(),
                               model_data->second.data(), model_data->second.size());

//  AsyncObjectDetector model_runner;
//  model_runner.model().loadFromBuffer(model_data->first.data(), model_data->first.size(),
//                                      model_data->second.data(), model_data->second.size());

  AsyncVideoClient video_client(url, port);

  bool stop = false;
  bool pause = false;
  int criteria = 50;

  Text text_fps;
  Text text_criteria;
  std::vector<std::pair<Text, Rectangle>> predict_result;

  const auto run_detection = [&] (cv::Mat image) {
    frames.store(std::move(image));
    updated = true;
//    frame = std::move(image);
  };
  auto conn = camera.add_listener(run_detection);
  camera.run();

  while(!stop) {
    if (!pause) {
      if (bool expected = true; !updated.compare_exchange_strong(expected, false)) {
        continue;
      }

      const auto frame_or_not = frames.load();
      if (!frame_or_not) {
        continue;
      }

      if (frame = *frame_or_not; frame.empty()) {
        continue;
      }

      cv::resize(frame, view, {}, 0.5, 0.5);

      detector.feed(frame, DateTime<>::now().milliseconds());
      if (auto result = detector.retrieve(); result) {
        for (const auto& detection : *result) {
          if (detection.score < criteria * 0.01) continue;

          const cv::Point2f tl(detection.rect[1] * view.cols, detection.rect[0] * view.rows);
          const cv::Point2f br(detection.rect[3] * view.cols, detection.rect[2] * view.rows);

          cv::rectangle(view, tl, br, {255, 0, 0}, 2);

          char buf[10];
          std::sprintf(buf, "(%.1f%%)", detection.score * 100);
          cv::putText(view,
                      detection.label + std::string(buf),
                      cv::Point2d(tl.x, tl.y - 4 * scale),
                      cv::FONT_ITALIC, 1 * scale, {255, 255, 255}, 3);
          cv::putText(view,
                      detection.label + std::string(buf),
                      cv::Point2d(tl.x, tl.y - 4 * scale),
                      cv::FONT_ITALIC, 1 * scale, {0, 0, 0}, 2);
        }
      }
    }

    cv::putText(view, "Criteria: " + std::to_string(criteria * 0.01), {0, 10 * (int)scale}, cv::FONT_HERSHEY_DUPLEX, 0.5 * scale, {0,255,0});

    const auto inference_time = detector.inference_time();
    cv::putText(view, "Inference: " + std::to_string(inference_time) + "ms", {0, 40 * (int)scale}, cv::FONT_HERSHEY_DUPLEX, 0.5 * scale, {0,255,0});

    const auto fps = camera.fps();
    cv::putText(view, "FPS: " + std::to_string(fps), {0, 20 * (int)scale}, cv::FONT_HERSHEY_DUPLEX, 0.5 * scale, {0,255,0});

    const auto now = DateTime<>::now().time_zone(std::chrono::hours(9)).to_string();
    cv::putText(view, now, {5, view.rows - 30 * int(scale)}, cv::FONT_HERSHEY_DUPLEX, 0.5 * scale, {200, 200, 200}, 1, cv::LINE_AA);

    video_client.feed(view, now, std::vector<std::string>());

# ifdef __APPLE__
    cv::imshow("Raspberry Pi", view);
    if (const auto key = cv::waitKey(16); key != -1) {
      std::cout << key << '(' << char(key) << ')' << '\n';
      switch(key) {
        case kESC: // ESC
          stop = true;
          break;

        case kSPACE: // SPACE
          pause = !pause;
          break;

        case '-':
          criteria = std::max(0, criteria - 10);
          break;

        case '+':
          criteria = std::min(100, criteria + 10);
          break;
      }
//      generator->handle_key(key);
    }
# endif
  }

  conn.disconnect();


  return 0;
}
