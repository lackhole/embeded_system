#include <atomic>
#include <iostream>
#include <stdexcept>
#include <string>
#include <memory>
#include <vector>

# ifdef __APPLE__
#include <algorithm>
# endif

#include "opencv2/opencv.hpp"

#include "watcher/camera/async_camera_controller.h"
#include "watcher/detector/movement_detector.h"
#include "watcher/detector/object_detection_model.h"
#include "watcher/drawable/drawable.h"
#include "watcher/network/async_video_client.h"
#include "watcher/utility/date_time.h"

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

  watcher::TcpClient client(url, port);
  watcher::Protocol protocol;

  const auto retry_get = [](int wait, auto func, auto&&... args) {
    do {
      try {
        return func(std::forward<decltype(args)>(args)...);
      } catch (const std::exception& e) {
        watcher::Log.e(e.what(), ". Retrying...");
        std::this_thread::sleep_for(std::chrono::milliseconds(wait));
      }
    } while (true);
  };

  for (;;) {
    auto response = retry_get(3000, [&]() {
      return protocol.Get(client, "model/model.tflite");
//      return protocol.Get(client, "model/ssd_mobilenet_v1_1_metadata_1.tflite");
//      return protocol.Get(client, "model/yolov4-416-fp16.tflite");
    });

    auto it = response.find("data");
    if (it == response.end()) {
      watcher::Log.d("Failed to load model");
      std::this_thread::sleep_for(std::chrono::seconds(3));
      continue;
    }

    model_buffer = std::move(it->second);
    break;
  }

  for (;;) {
    auto response = retry_get(3000, [&]() {
      return protocol.Get(client, "model/labelmap.txt");
//      return protocol.Get(client, "model/labelmap_ssd.txt");
//      return protocol.Get(client, "model/labelmap_yolov4.txt");
    });

    auto it = response.find("data");
    if (it == response.end()) {
      watcher::Log.d("Failed to load labelmap");
      std::this_thread::sleep_for(std::chrono::seconds(3));
      continue;
    }

    labelmap_buffer = std::move(it->second);
    break;
  }

  return std::make_pair(std::move(model_buffer), std::move(labelmap_buffer));
}

std::string remove_trailing(const std::string& s) {
  std::string r = s;
  if (!r.empty() &&
    (r[r.size() - 1] == '\r' || r[r.size() - 1] == '\n' ||
      r[r.size() - 1] == ' ' || r[r.size() - 1] == '\t' ||
      r[r.size() - 1] == '\0'))
    r.erase(r.size() - 1);

  return std::move(r);
}

bool run(const std::string& url, const std::string& port) {
  cv::Mat view;
  watcher::RingBuffer<cv::Mat> frames;
  std::atomic<bool> updated{false};
  cv::Mat frame;
  cv::Mat frame_prev;
  watcher::AsyncCameraController camera;
  camera.open();
  if (!camera.is_open()) {
    return EXIT_FAILURE;
  }


  const auto model_data = load_model_data(url, port);
  if (!model_data)
    return EXIT_FAILURE;

  watcher::MovementDetector detector;
  detector.LoadModelFromBuffer(model_data->first.data(), model_data->first.size(),
                               model_data->second.data(), model_data->second.size());

  std::mutex inference_result_m;
  watcher::MovementDetector::result_or_not inference_result;
  boost::signals2::scoped_connection conn_detector = detector.add_listener([&](const auto& result) {
    std::lock_guard lck(inference_result_m);
    inference_result = result;
  });

//  AsyncObjectDetector model_runner;
//  model_runner.model().loadFromBuffer(model_data->first.data(), model_data->first.size(),
//                                      model_data->second.data(), model_data->second.size());

  std::atomic<bool> restart{false};

  watcher::AsyncVideoClient video_client(url, port);
  boost::signals2::scoped_connection conn_g = video_client.AddGetListener([&](auto response) {
    auto it = response.find("data");
    if (it == response.end()) {
      return;
    }
    const auto r = remove_trailing(it->second);
    watcher::Log.d("Loaded option settings/restart : ", r);

    restart = (r == "1");
  }, "settings/restart");

  boost::signals2::scoped_connection conn_s = video_client.AddGetListener([&](auto response) {
    auto it = response.find("data");
    if (it == response.end()) {
      return;
    }
    const auto r = remove_trailing(it->second);

    try {
      const auto s = std::stof(r);
      if (0 <= s && s <= 1)
        detector.score_threshold(s);
    } catch (...) {}

  }, "settings/score");

  bool stop = false;
  bool pause = false;

  const double scale = 1;

  watcher::Text text_fps = watcher::Text()
    .org({0, 25 * (int)scale})
    .color({0,255,0})
    .font_face(cv::FONT_HERSHEY_DUPLEX)
    .font_scale(scale * 0.5);
  watcher::Text text_criteria = watcher::Text()
    .org({0, 10 * (int)scale})
    .color({0,255,0})
    .font_face(cv::FONT_HERSHEY_DUPLEX)
    .font_scale(scale * 0.5);
  watcher::Text text_inference = watcher::Text()
    .org({0, 40 * (int)scale})
    .color({0,255,0})
    .font_face(cv::FONT_HERSHEY_DUPLEX)
    .font_scale(scale * 0.5);
  watcher::Text text_time = watcher::Text()
    .color({200,200,200})
    .font_face(cv::FONT_HERSHEY_DUPLEX)
    .font_scale(scale * 0.5)
    .thickness(1)
    .line_type(cv::LINE_AA);

  const auto run_detection = [&] (cv::Mat image) {
    frames.store(std::move(image));
    updated = true;
//    frame = std::move(image);
  };
  auto conn = camera.add_listener(run_detection);
  camera.run();

  std::mutex bbox_m;
  std::vector<cv::Rect> bbox;
  detector.bbox_.connect([&](const auto& b) { std::lock_guard lck(bbox_m); bbox = b; });

  while(!stop) {
    if (restart)
      return true;

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

      detector.feed(frame, watcher::DateTime<>::now().milliseconds());

      decltype(inference_result) result_copy;
      {
        std::lock_guard lck(inference_result_m);
        result_copy = inference_result;
      }
      if (result_copy) {
        for (const auto& detection: *result_copy) {
          const cv::Point2f tl(detection.rect[1] * view.cols, detection.rect[0] * view.rows);
          const cv::Point2f br(detection.rect[3] * view.cols, detection.rect[2] * view.rows);

          cv::rectangle(view, tl, br, {255, 0, 0}, 2);

          char buf[10];
          std::sprintf(buf, "(%.1f%%)", detection.score * 100);
          cv::putText(view,
                      detection.label + std::string(buf),
                      cv::Point2d(tl.x, tl.y - 4 * scale),
                      cv::FONT_ITALIC, 0.5 * scale, {255, 255, 255}, 2);
          cv::putText(view,
                      detection.label + std::string(buf),
                      cv::Point2d(tl.x, tl.y - 4 * scale),
                      cv::FONT_ITALIC, 0.5 * scale, {0, 0, 0}, 1);
        }
      }
    }
    const auto now = watcher::DateTime<>::now().time_zone(std::chrono::hours(9)).to_string();

    text_criteria.text("Criteria: " + std::to_string(detector.score_threshold()));
    text_inference.text("Inference: " + std::to_string(detector.inference_time()) + "ms");
    text_fps.text("FPS: " + std::to_string(camera.fps()));
    text_time.text(now)
      .org({5, view.rows - 30 * int(scale)});

    decltype(bbox) bbox_copy;
    {
      std::lock_guard lck(bbox_m);
      bbox_copy = bbox;
    }
    for (const auto& rect : bbox_copy) {
      cv::rectangle(view, cv::Point(rect.tl() / 2), cv::Point(rect.br()/2), {0,0,220}, 1);
    }

    watcher::draw(view, text_criteria, text_inference, text_fps, text_time);

    video_client.feed(view, now, std::vector<std::string>());

# ifdef __APPLE__
    cv::imshow("Raspberry Pi", view);
    if (const auto key = cv::waitKey(16); key != -1) {
      watcher::Log.d(key, '(', char(key), ')');
      switch(key) {
        case kESC: // ESC
          stop = true;
          break;

        case kSPACE: // SPACE
          pause = !pause;
          break;

        case '-':
          detector.score_threshold(std::max(0.f, detector.score_threshold() - 0.1f));
          break;

        case '+':
          detector.score_threshold(std::min(1.f, detector.score_threshold() + 0.1f));
          break;
      }
//      generator->handle_key(key);
    }
# endif
  }

  conn.disconnect();

  return false;
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

  while (true) {
    if (run(url, port)) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
      continue;
    }
    break;
  }

  return 0;
}
