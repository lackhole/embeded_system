//
// Created by YongGyu Lee on 2022/06/10.
//

#include "embed/network/async_video_client.h"

#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>

#include "opencv2/opencv.hpp"

#include "embed/utility/date_time.h"

void AsyncVideoClient::feed(cv::Mat image, std::string timestamp) {
  input_.store(std::move(image), std::move(timestamp));
  async_runner_.run();
}

void AsyncVideoClient::OnWakeUp() {
  const auto input = input_.load();
  if (input) {
    const auto image = input->get().first;
    auto timestamp = input->get().second;

    // TODO: Write to packet directly
    std::vector<uchar> buf;
    cv::imencode(".jpg", image, buf);

    try {
      protocol_.Post(
        client_,
        buf,
        Protocol::key_value_pair({
          {"Timestamp", std::move(timestamp)},
          {"FileFormat", ".jpg"}
        }));
    } catch (const std::exception& e) {
      std::cerr << DateTime<>::now(std::chrono::hours(9)).to_string() << " | " << e.what() << '\n';
    }
  }
}
