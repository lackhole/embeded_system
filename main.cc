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
//#include "input/camera_input.h"
//#include "input/image_input.h"
//#include "input/video_input.h"

constexpr auto kPWD = "/Users/yonggyulee/CLionProjects/embed/model/";

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


  cv::Mat frame;
  AsyncCameraController camera;

  ObjectDetectionModel model(std::string(kPWD) + "/ssd_mobilenet_v1_1_metadata_1.tflite",
                             std::string(kPWD) + "/labelmap.txt");

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
                    cv::Point2d{rects[i][1] * frame.cols, rects[i][0] * frame.rows - 4},
                    cv::FONT_ITALIC, 1, {255, 255, 255}, 3);
        cv::putText(frame,
                    label[i] + std::string(buf),
                    cv::Point2d{rects[i][1] * frame.cols, rects[i][0] * frame.rows - 4},
                    cv::FONT_ITALIC, 1, {0, 0, 0}, 2);
      }
      cv::putText(frame, "Criteria: " + std::to_string(criteria * 0.01), {0, 10}, cv::FONT_HERSHEY_DUPLEX, 0.5, {0,255,0});
    }

    cv::imshow("Window", frame);
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
