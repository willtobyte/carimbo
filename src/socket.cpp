#include "socket.hpp"

using namespace network;

using json = nlohmann::json;

#ifdef EMSCRIPTEN
EM_BOOL websocket_on_open(int, const EmscriptenWebSocketOpenEvent *event, void *data) {
  auto *self = static_cast<socket *>(data);
  if (!self) return EM_FALSE;

  self->handle_open(event);
  return EM_FALSE;
}

EM_BOOL websocket_on_message(int, const EmscriptenWebSocketMessageEvent *event, void *data) {
  auto *self = static_cast<socket *>(data);
  if (!self) return EM_FALSE;

  self->handle_message(event);
  return EM_FALSE;
}

EM_BOOL websocket_on_error(int, const EmscriptenWebSocketErrorEvent *event, void *data) {
  auto *self = static_cast<socket *>(data);
  if (!self) return EM_FALSE;

  self->handle_error(event);
  return EM_FALSE;
}

EM_BOOL websocket_on_close(int, const EmscriptenWebSocketCloseEvent *event, void *data) {
  auto *self = static_cast<socket *>(data);
  if (!self) return EM_FALSE;

  self->handle_close(event);
  return EM_FALSE;
}
#endif

socket::socket() noexcept {
  _queue.reserve(8);

#ifdef EMSCRIPTEN
  const std::string url =
#ifdef LOCAL
      "http://localhost:3000/socket";
#else
      "https://" + std::string(emscripten_run_script_string("window.location.hostname")) + "/socket";
#endif
  EmscriptenWebSocketCreateAttributes attrs = {
      url.c_str(),
      nullptr,
      true
  };

  _socket = emscripten_websocket_new(&attrs);
  if (_socket <= 0) {
    invoke("error", "Failed to create WebSocket");
    return;
  }

  emscripten_websocket_set_onopen_callback(_socket, this, websocket_on_open);
  emscripten_websocket_set_onmessage_callback(_socket, this, websocket_on_message);
  emscripten_websocket_set_onerror_callback(_socket, this, websocket_on_error);
  emscripten_websocket_set_onclose_callback(_socket, this, websocket_on_close);
#endif
}

socket::~socket() noexcept {
#ifdef EMSCRIPTEN
  constexpr int code = 1000;
  constexpr const char *reason = "Client disconnecting";

  if (_socket) {
    emscripten_websocket_close(_socket, code, reason);
    emscripten_websocket_delete(_socket);
    _socket = 0;
  }
#endif
}

void socket::emit(const std::string &topic, const std::string &data) noexcept {
  std::ostringstream oss;
  oss << R"({"event": {"topic": ")" << topic << R"(", "data": )" << data << R"(}})";
  send(oss.str());
}

void socket::on(const std::string &topic, std::function<void(const std::string &)> callback) noexcept {
  std::ostringstream oss;
  oss << R"({"subscribe": ")" << topic << R"("})";
  send(oss.str());
  _callbacks[topic].push_back(std::move(callback));
}

void socket::rpc(const std::string &method, const std::string &arguments, std::function<void(const std::string &)> callback) noexcept {
  std::ostringstream oss;
  oss << R"({"rpc": {"request": {"id": )" << ++counter
      << R"(, "method": ")" << method
      << R"(", "arguments": )" << arguments << R"(}}})";
  send(oss.str());

  _callbacks[method].push_back(std::move(callback));
}

#ifdef EMSCRIPTEN
void socket::handle_open(const EmscriptenWebSocketOpenEvent *event) {
  UNUSED(event);
  _connected = true;

  for (const auto &message : _queue) {
    send(message);
  }
  _queue.clear();
}

void socket::handle_message(const EmscriptenWebSocketMessageEvent *event) {
  if (!event->isText) {
    return;
  }

  std::string buffer(reinterpret_cast<const char *>(event->data), event->numBytes);

  on_message(buffer);
}

void socket::handle_error(const EmscriptenWebSocketErrorEvent *event) {
  UNUSED(event);
  invoke("error", "WebSocket error occurred");
}

void socket::handle_close(const EmscriptenWebSocketCloseEvent *event) {
  UNUSED(event);
}
#endif

void socket::on_message(const std::string &buffer) noexcept {
  auto j = json::parse(buffer, nullptr, false);
  if (j.is_discarded()) {
    return;
  }

  if (j.value("command", "") == "ping") {
    send(R"({"command": "pong"})");
    return;
  }

  if (j.value("command", "") == "reload") {
#ifdef EMSCRIPTEN
    emscripten_run_script_string("window.location.reload()");
#endif
    return;
  }

  if (auto event = j.value("event", json::object()); !event.empty()) {
    invoke(
        event.at("topic").get_ref<const std::string &>(),
        event.at("data").dump()
    );

    return;
  }

  if (auto rpc = j.value("rpc", json::object()); !rpc.empty() && rpc.contains("response")) {
    auto response = rpc.at("response");
    if (response.contains("result")) {
      invoke(
          std::to_string(response.at("id").get<uint64_t>()),
          response.at("result").dump()
      );
    }

    if (response.contains("error")) {
      // TODO handle error
    }

    return;
  }
}

void socket::send(const std::string &message) noexcept {
  if (!_connected) {
    _queue.emplace_back(message);
    return;
  }

#ifdef EMSCRIPTEN
  emscripten_websocket_send_utf8_text(_socket, message.c_str());
#endif
}

void socket::invoke(const std::string &event, const std::string &data) const noexcept {
  if (auto it = _callbacks.find(event); it != _callbacks.end()) {
    for (const auto &callback : it->second) {
      callback(data);
    }
  }
}
