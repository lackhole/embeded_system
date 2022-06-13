//
// Created by YongGyu Lee on 2022/06/10.
//

#include "embed/network/tcp_client.h"

#include <string>

#include "boost/asio.hpp"

TcpClient::TcpClient(const std::string& url, const std::string& port)
  : resolver_(io_service_),
    query_(url, port),
    endpoints_(resolver_.resolve(query_)),
    socket_(io_service_)
{}

void TcpClient::close() {
  if (socket_.is_open())
    socket_.close();
}

size_t TcpClient::send(const char* data, size_t size) {
  boost::system::error_code ignored_error;

  boost::asio::connect(socket_, endpoints_);
  return boost::asio::write(socket_, boost::asio::buffer(data, size), ignored_error);
}

size_t TcpClient::receive(char* dst, size_t max_size, boost::system::error_code error) {
  boost::asio::connect(socket_, endpoints_);
  size_t len = boost::asio::read(socket_, boost::asio::buffer(dst, max_size), error);
  return len;
}
