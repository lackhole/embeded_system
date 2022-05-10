#include <iostream>
#include <filesystem>
#include <string>
#include <memory>
#include <fstream>
#include <vector>

#include "opencv2/opencv.hpp"

#include "tensorflow/lite/c/c_api.h"
#include "tensorflow/lite/c/common.h"

#include "cutemodel/cute_model.h"

#include "embed/object_detection_model.h"
#include "embed/async_camera_controller.h"
#include "embed/date_time.h"
//#include "input/camera_input.h"
//#include "input/image_input.h"
//#include "input/video_input.h"

#if __linux__
constexpr auto kPWD = "/home/pi/embeded_system";
#else
constexpr auto kPWD = "/Users/yonggyulee/CLionProjects/embed/model";
#endif

enum Key {
  kLEFT = 2,
  kRIGHT = 3,
  kTAB = 9,
  kESC = 27,
  kSPACE = 32,
};

int main() {
  namespace fs = std::filesystem;
  std::cout << fs::current_path() << std::endl;

  cv::Mat view;
  cv::Mat frame;
  AsyncCameraController camera;

  const double scale = 2;

  ObjectDetectionModel model(std::string(kPWD) + "/model/ssd_mobilenet_v1_1_metadata_1.tflite",
                             std::string(kPWD) + "/model/labelmap.txt");

  bool stop = false;
  bool pause = false;
  int criteria = 50;

  while(!stop) {
    if (!pause) {
      camera >> frame;
      if (frame.empty()) {
        continue;
      }

      const auto [rects, label, scores, numDetect] = model.invoke(frame);

      for (int i = 0; i < numDetect; ++i) {
        if (scores[i] < criteria * 0.01) break;
        cv::Point2d tl{rects[i][1] * frame.cols, rects[i][0] * frame.rows};
        cv::Point2d br{rects[i][3] * frame.cols, rects[i][2] * frame.rows};

        cv::rectangle(frame, tl, br, {255, 0, 0}, 2);

        char buf[10];
        std::sprintf(buf, "(%.1f%%)", scores[i] * 100);
        cv::putText(frame,
                    label[i] + std::string(buf),
                    cv::Point2d{rects[i][1] * frame.cols, rects[i][0] * frame.rows - 4 * scale},
                    cv::FONT_ITALIC, 1 * scale, {255, 255, 255}, 3);
        cv::putText(frame,
                    label[i] + std::string(buf),
                    cv::Point2d{rects[i][1] * frame.cols, rects[i][0] * frame.rows - 4 * scale},
                    cv::FONT_ITALIC, 1 * scale, {0, 0, 0}, 2);
      }
      cv::putText(frame, "Criteria: " + std::to_string(criteria * 0.01), {0, 10 * (int)scale}, cv::FONT_HERSHEY_DUPLEX, 0.5 * scale, {0,255,0});
    }
    const auto fps = camera.fps();
    cv::putText(frame, "FPS: " + std::to_string(fps), {0, 20 * (int)scale}, cv::FONT_HERSHEY_DUPLEX, 0.5 * scale, {0,255,0});

    const auto now = DateTime<>::now().time_zone(std::chrono::hours(9)).to_string();
    cv::putText(frame, now, {5, frame.rows - 30 * int(scale)}, cv::FONT_HERSHEY_DUPLEX, 0.5 * scale, {200, 200, 200}, 1, cv::LINE_AA);

    cv::resize(frame, view, {}, 0.5, 0.5);
    cv::imshow("Window", view);
    if (const auto key = cv::waitKey(16); key != -1) {
      std::cout << key << '(' << char(key) << ')' << '\n';
      switch(key) {
        case kESC: // ESC
          stop = true;
          break;

        case kSPACE: // SPACE
          pause = !pause;

        case '-':
          criteria = std::max(0, criteria - 10);
          break;

        case '+':
          criteria = std::min(100, criteria + 10);
      }
//      generator->handle_key(key);
    }
  }


  return 0;
}
