#pragma once

#include "common.hpp"

namespace network {
#ifndef EMSCRIPTEN
namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace net = boost::asio;
namespace ssl = net::ssl;
using tcp = net::ip::tcp;
#endif

class socket {
public:
  socket() noexcept;
  ~socket() noexcept;

  void connect() noexcept;

  void emit(const std::string &topic, const std::string &data) noexcept;
  void on(const std::string &topic, std::function<void(const std::string &)> callback) noexcept;
  void rpc(const std::string &method, const std::string &arguments, std::function<void(const std::string &)> callback) noexcept;

#ifdef EMSCRIPTEN
  void handle_open(const EmscriptenWebSocketOpenEvent *event);
  void handle_message(const EmscriptenWebSocketMessageEvent *event);
  void handle_error(const EmscriptenWebSocketErrorEvent *event);
  void handle_close(const EmscriptenWebSocketCloseEvent *event);
#endif
private:
#ifndef EMSCRIPTEN
  void on_resolve(beast::error_code ec, tcp::resolver::results_type results) noexcept;
  void on_connect(beast::error_code ec, const tcp::resolver::results_type::endpoint_type &endpoint) noexcept;
  void on_ssl_handshake(beast::error_code ec) noexcept;
  void on_handshake(beast::error_code ec) noexcept;
  void on_read(beast::error_code ec, std::size_t bytes_transferred) noexcept;
  void do_read();
#endif
  void on_message(const std::string &buffer) noexcept;
  void send(const std::string &message) noexcept;
  void invoke(const std::string &event, const std::string &data = "{}") const noexcept;

  bool _connected{false};
  std::atomic<uint64_t> counter{0};
  std::vector<std::string> _queue;
  std::unordered_map<std::string, std::vector<std::function<void(const std::string &)>>> _callbacks;

#ifdef EMSCRIPTEN
  EMSCRIPTEN_WEBSOCKET_T _socket{0};
#else
  net::io_context _io_context;
  boost::asio::executor_work_guard<net::io_context::executor_type> _work_guard;
  tcp::resolver _resolver;
  boost::asio::ssl::context _ssl_context;
  websocket::stream<ssl::stream<beast::tcp_stream>> _ws;
  beast::flat_buffer _buffer;
  std::function<void(const std::string &)> _on_message;
  std::thread _thread;
#endif
};
}
