//
// Created by YongGyu Lee on 2022/06/11.
//

#ifndef EMBED_NETWORK_PROTOCOL_H_
#define EMBED_NETWORK_PROTOCOL_H_

#include <cstddef>
#include <chrono>
#include <functional>
#include <iostream>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>
#include <unordered_map>
#include <vector>

#include "embed/network/packet.h"
#include "embed/utility/frequency.h"
#include "embed/utility/logger.h"


inline auto get_ms() {
  const auto now = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
}

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
  Protocol(size_t packet_cap) : packet_(packet_cap) {}

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
    client.close();

    packet_.clear();
    const auto len = client.receive(packet_.buffer(), packet_.capacity());
    packet_.setSize(len);
    Log.d("Got ", packet_.size(), "bytes from the server.");

    std::unordered_map<std::string, std::string> result;

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
    const auto t1 = get_ms();

    size_t sent_size_data = 0;
    size_t remaining_size = data_size;
    size_t sent_size_packet = 0;

    while (remaining_size > 0) {
      const auto sending_size = kPacketSize > remaining_size ? remaining_size : kPacketSize;

      // TODO: Send some header values only once at the beginning
      key_value_pair header = {
        {kRequest, kRequestPost},
        {kHeaderTotalSize, std::to_string(data_size)},
        {kHeaderDone, std::to_string(sending_size == remaining_size)}
      };
      if (optional_header) {
        header.merge(std::move(*optional_header));
      }

      packet_.clear();
      packet_.write_header(header)
        .write_data(data + sent_size_data, sending_size);

      client.send(packet_.buffer(), packet_.size());

      remaining_size -= sending_size;
      sent_size_data += sending_size;
      sent_size_packet += packet_.size();
      Log.d("Sent ", sending_size, "bytes. (", sending_size, '/', data_size, ')');
    }
    client.close();
    const auto t2 = get_ms();

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


#endif // EMBED_NETWORK_PROTOCOL_H_
