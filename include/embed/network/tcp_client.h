//
// Created by YongGyu Lee on 2022/06/10.
//

#ifndef EMBED_NETWORK_ASYNC_TCP_CLIENT_H_
#define EMBED_NETWORK_ASYNC_TCP_CLIENT_H_

#include <cstddef>
#include <string>
#include <thread>

#include "boost/asio.hpp"
#include "opencv2/opencv.hpp"

#include "embed/utility/ring_buffer.h"

class TcpClient {
 public:
  TcpClient(const std::string& url, int port) : TcpClient(url, std::to_string(port)) {}
  TcpClient(const std::string& url, const std::string& port);

  ~TcpClient() { close(); }

  size_t send(const char* data, size_t size);

  size_t receive(char* dst, size_t max_size);

  void close();

 private:
  using boost_tcp = boost::asio::ip::tcp;

  boost::asio::io_service io_service_;
  boost_tcp::resolver resolver_;
  boost_tcp::resolver::query query_;
  boost_tcp::resolver::results_type endpoints_;
  boost_tcp::socket socket_;
};


#endif // EMBED_NETWORK_ASYNC_TCP_CLIENT_H_
