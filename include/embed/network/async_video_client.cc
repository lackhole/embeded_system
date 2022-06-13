//
// Created by YongGyu Lee on 2022/06/10.
//

#include "embed/network/async_video_client.h"

#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>

#include "opencv2/opencv.hpp"

#include "embed/utility/logger.h"

void AsyncVideoClient::feed(cv::Mat image, std::string timestamp,
                            std::vector<std::string> detected_object) {
  input_.store(std::move(image), std::move(timestamp), std::move(detected_object));
  async_runner_.run();
}

void AsyncVideoClient::OnWakeUp() {
  const auto input = input_.load();
  if (input) {
    const auto image = std::get<0>(*input);
    auto timestamp = std::get<1>(*input);
    const auto& objs = std::get<2>(*input);

    std::string s;
    for (const auto& obj : objs)
      s += obj + ",";

    try {
      // TODO: Write to packet directly
      std::vector<uchar> buf;
      cv::imencode(".jpg", image, buf);

      protocol_.Post(
        client_,
        buf,
        Protocol::key_value_pair({
          {"Timestamp", std::move(timestamp)},
          {"FileFormat", ".jpg"},
          {"Objects", "\'" + s + "\'"}
        }));
    } catch (const std::exception& e) {
      Log.e(e.what());
    }
  }
}
