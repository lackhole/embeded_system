//
// Created by YongGyu Lee on 2022/06/10.
//

#ifndef WATCHER_NETWORK_ASYNC_TCP_CLIENT_H_
#define WATCHER_NETWORK_ASYNC_TCP_CLIENT_H_

#include <cstddef>
#include <string>

#include "boost/asio.hpp"

namespace watcher {

class TcpClient {
 public:
  TcpClient(const std::string& url, int port) : TcpClient(url, std::to_string(port)) {}
  TcpClient(const std::string& url, const std::string& port);

  ~TcpClient() { close(); }

  size_t send(const char* data, size_t size);

  size_t receive(char* dst, size_t max_size, boost::system::error_code error);

  void close();

 private:
  using boost_tcp = boost::asio::ip::tcp;

  boost::asio::io_service io_service_;
  boost_tcp::resolver resolver_;
  boost_tcp::resolver::query query_;
  boost_tcp::resolver::results_type endpoints_;
  boost_tcp::socket socket_;
};

} // namespace watcher

#endif // WATCHER_NETWORK_ASYNC_TCP_CLIENT_H_
