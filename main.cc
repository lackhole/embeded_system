#include <iostream>
#include <string>
#include <memory>
#include <vector>

#include "opencv2/opencv.hpp"

#include "embed/camera/async_camera_controller.h"
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

void run(std::string url, std::string port) {

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
  cv::Mat frame;
  cv::Mat frame_prev;
  AsyncCameraController camera;
  camera.open();
  if (!camera.is_open()) {
    return EXIT_FAILURE;
  }

  const double scale = 2;

  AsyncObjectDetector model_runner;

  model_runner.model().load(std::string(kPWD) + "/model/ssd_mobilenet_v1_1_metadata_1.tflite",
//  model_runner.model().load(std::string(kPWD) + "/model/quantized.tflite",
                             std::string(kPWD) + "/model/labelmap.txt");

  AsyncVideoClient video_client(url, port);

  bool stop = false;
  bool pause = false;
  int criteria = 50;

  Text text_fps;
  Text text_criteria;
  std::vector<std::pair<Text, Rectangle>> predict_result;


  const auto run_detection = [&] (cv::Mat image) {
    frames.store(std::move(image));
//    frame = std::move(image);
  };
  auto conn = camera.add_listener(run_detection);
  camera.run();

  while(!stop) {
    if (!pause) {
      const auto frame_or_not = frames.load();
      if (!frame_or_not) {
        continue;
      }
      frame = *frame_or_not;
      if (frame.empty()) {
        continue;
      }

      model_runner.feed(frame);
      if (auto result = model_runner.retrieve(); result) {
        const auto fps = model_runner.fps();
        cv::putText(frame, "Inference: " + std::to_string(1000/fps) + "ms", {0, 40 * (int)scale}, cv::FONT_HERSHEY_DUPLEX, 0.5 * scale, {0,255,0});


        for (const auto& detection : *result) {
          if (detection.score < criteria * 0.01) continue;

          const cv::Point2f tl(detection.rect[1] * frame.cols, detection.rect[0] * frame.rows);
          const cv::Point2f br(detection.rect[3] * frame.cols, detection.rect[2] * frame.rows);

          cv::rectangle(frame, tl, br, {255, 0, 0}, 2);

          char buf[10];
          std::sprintf(buf, "(%.1f%%)", detection.score * 100);
          cv::putText(frame,
                      detection.label + std::string(buf),
                      cv::Point2d(tl.x, tl.y - 4 * scale),
                      cv::FONT_ITALIC, 1 * scale, {255, 255, 255}, 3);
          cv::putText(frame,
                      detection.label + std::string(buf),
                      cv::Point2d(tl.x, tl.y - 4 * scale),
                      cv::FONT_ITALIC, 1 * scale, {0, 0, 0}, 2);
        }
      }
    }
    cv::putText(frame, "Criteria: " + std::to_string(criteria * 0.01), {0, 10 * (int)scale}, cv::FONT_HERSHEY_DUPLEX, 0.5 * scale, {0,255,0});
//    text_criteria.text("Criteria: " + std::to_string(criteria * 0.01));
    const auto fps = camera.fps();
//    text_fps.text("FPS: " + std::to_string(fps));
    cv::putText(frame, "FPS: " + std::to_string(fps), {0, 20 * (int)scale}, cv::FONT_HERSHEY_DUPLEX, 0.5 * scale, {0,255,0});

    const auto now = DateTime<>::now().time_zone(std::chrono::hours(9)).to_string();
    cv::putText(frame, now, {5, frame.rows - 30 * int(scale)}, cv::FONT_HERSHEY_DUPLEX, 0.5 * scale, {200, 200, 200}, 1, cv::LINE_AA);

    cv::resize(frame, view, {}, 0.5, 0.5);

    video_client.feed(view, now);

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
# endif
    }
  }

  conn.disconnect();


  return 0;
}
