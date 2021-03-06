//
// Created by YongGyu Lee on 2022/06/11.
//

#ifndef WATCHER_NETWORK_PROTOCOL_H_
#define WATCHER_NETWORK_PROTOCOL_H_

#include <cstddef>
#include <chrono>
#include <optional>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>

#include "boost/asio.hpp"

#include "watcher/network/packet.h"
#include "watcher/utility/date_time.h"
#include "watcher/utility/frequency.h"
#include "watcher/utility/logger.h"

namespace watcher {

class Protocol {
 public:
  template<typename T, typename = void>
  struct has_data : std::false_type {};
  template<typename T>
  struct has_data<T, std::void_t<decltype(std::declval<const T&>().data())>> : std::true_type {};
  template<typename T, typename = void>
  struct has_size : std::false_type {};
  template<typename T>
  struct has_size<T, std::void_t<decltype(std::declval<const T&>().size())>> : std::true_type {};
  template<typename T>
  struct data_is_ptr : std::is_pointer<decltype(std::declval<const T&>().data())> {};
  template<typename T>
  struct valid_container : std::conjunction<has_data<T>, has_size<T>, data_is_ptr<T>> {};

  Protocol() = default;

  // TODO: Support file
  enum DataType {
    kBuffer,
    kFile,
  };

  enum RequestType {
    kGet,
    kPost,
  };

  using key_value_pair = std::unordered_map<std::string, std::string>;
  using response = key_value_pair;
  using optional_header = std::optional<key_value_pair>;

  template<typename Client>
  [[nodiscard]] response Get(Client& client, const char* request, size_t data_size) {
    // TODO: Check size
    packet_
      .write_header({
        {kRequest, kRequestGet},
        {kHeaderDone, "1"}
      })
      .write_data(request, data_size);

    client.send(packet_.buffer(), packet_.size());
    Log.d("Sent ", packet_.size(), "bytes to the server.");

    std::unordered_map<std::string, std::string> result;
    boost::system::error_code error;

    for (;;) {
      packet_.clear();
      const auto len = client.receive(packet_.buffer(), packet_.capacity(), error);
      packet_.setSize(len);

      const auto header = packet_.header();
      const auto it = header.find(kHeaderDone);
      const auto done = it != header.end() && it->second == "1";

      Log.d("Got ", len, "bytes from the server.");

      // TODO: Use status code check
      if (len < to_byte(kPacketHeaderSizeBit))
        break;

      for (const auto& p : header) {
        result.emplace(p.first, p.second);
      }

      result["data"].insert(result["data"].end(), packet_.data().first, packet_.data().first + packet_.data().second);

      if (done) {
        break;
      }
      if (error == boost::asio::error::eof) {
        if (auto d = result.find("data"); d != result.end())
          result.erase(d);
        break;
      }
    }

    if (error == boost::asio::error::eof) {
      watcher::Log.e("EOF: Connection closed cleanly by peer.");
    }

    return result;
  }

  template<typename Client, typename T>
  response Get(Client& client, const T* data, size_t data_size) {
    return Get(client, reinterpret_cast<const char*>(data), sizeof(T) * data_size);
  }

  template<typename Client, typename Container, typename T = typename Container::value_type,
    std::enable_if_t<valid_container<Container>::value, int> = 0>
  response Get(Client& client, const Container& data) {
    return Get(client, data.data(), data.size());
  }

  template<typename Client, typename T, size_t N>
  response Get(Client& client, const T(&arr)[N]) {
    return Get(client, arr, N);
  }

  template<typename Client>
  void Post(Client& client, const char* data, size_t data_size,
            optional_header optional_header = std::nullopt) {
    const auto t1 = DateTime<>::now().milliseconds();

    size_t sent_size_data = 0;
    size_t remaining_size = data_size;
    size_t sent_size_packet = 0;

    while (remaining_size > 0) {
      // TODO: Send some header values only once at the beginning
      key_value_pair header = {
        {kRequest, kRequestPost},
        {kHeaderTotalSize, std::to_string(data_size)},
        {kHeaderDone, "0"}
      };
      if (optional_header) {
        header.merge(std::move(*optional_header));
      }

      const auto header_size = Packet::CalcHeaderSize(header);
      header[kHeaderDone] = std::to_string((header_size + remaining_size) <= packet_.capacity());

      packet_.clear();
      packet_.write_header(header);

      const auto sending_size = packet_.remaining_size() > remaining_size ? remaining_size : packet_.remaining_size();

      packet_.write_data(data + sent_size_data, sending_size);

      client.send(packet_.buffer(), packet_.size());

      remaining_size -= sending_size;
      sent_size_data += sending_size;
      sent_size_packet += packet_.size();
      Log.d("Sent ", sending_size, "bytes. (", sending_size, '/', data_size, ')');
    }
    client.close();
    const auto t2 = DateTime<>::now().milliseconds();

    const auto bit_per_sec = static_cast<double>(sent_size_packet) / (t2 - t1) * 1000;

    Log.d(bit_per_sec * 0.000'001, "MB/s");
  }

  template<typename Client, typename T>
  void Post(Client& client, const T* data, size_t data_size,
            optional_header optional_header = std::nullopt) {
    Post(client, reinterpret_cast<const char*>(data), sizeof(T) * data_size, std::move(optional_header));
  }

  template<typename Client, typename Container, typename T = typename Container::value_type,
    std::enable_if_t<valid_container<Container>::value, int> = 0>
  void Post(Client& client, const Container& data,
            optional_header optional_header = std::nullopt) {
    Post(client, data.data(), data.size(), std::move(optional_header));
  }

  template<typename Client, typename T, size_t N>
  void Post(Client& client, const T(&req)[N],
            optional_header optional_header = std::nullopt) {
    Post(client, req, N, std::move(optional_header));
  }

 private:
  Packet packet_;
};

} // namespace watcher

#endif // WATCHER_NETWORK_PROTOCOL_H_
