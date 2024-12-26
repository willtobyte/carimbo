#pragma once

#include "common.hpp"

namespace network {
class socket {
public:
  socket() noexcept;
  ~socket() noexcept;

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
  void on_message(const std::string &buffer) noexcept;
  void send(const std::string &message) noexcept;
  void invoke(const std::string &event, const std::string &data = "{}") const noexcept;

  bool _connected{false};
  std::atomic<uint64_t> counter{0};
  std::vector<std::string> _queue;
  std::unordered_map<std::string, std::vector<std::function<void(const std::string &)>>> _callbacks;

#ifdef EMSCRIPTEN
  EMSCRIPTEN_WEBSOCKET_T _socket{0};
#endif
};
}
