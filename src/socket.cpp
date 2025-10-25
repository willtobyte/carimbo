#include "socket.hpp"

using namespace network;

using json = nlohmann::json;

#ifdef EMSCRIPTEN
EM_BOOL websocket_on_open(int, const EmscriptenWebSocketOpenEvent* event, void* data) {
  auto* self = static_cast<socket*>(data);
  if (!self) {
    return EM_FALSE;
  }

  self->handle_open(event);
  return EM_FALSE;
}

EM_BOOL websocket_on_message(int, const EmscriptenWebSocketMessageEvent* event, void* data) {
  auto* self = static_cast<socket*>(data);
  if (!self) {
    return EM_FALSE;
  }

  self->handle_message(event);
  return EM_FALSE;
}

EM_BOOL websocket_on_error(int, const EmscriptenWebSocketErrorEvent* event, void* data) {
  auto* self = static_cast<socket*>(data);
  if (!self) {
    return EM_FALSE;
  }

  self->handle_error(event);
  return EM_FALSE;
}

EM_BOOL websocket_on_close(int, const EmscriptenWebSocketCloseEvent* event, void* data) {
  auto* self = static_cast<socket*>(data);
  if (!self) {
    return EM_FALSE;
  }

  self->handle_close(event);
  return EM_FALSE;
}
#endif

socket::socket() {
  _queue.reserve(64);
#ifndef EMSCRIPTEN
#ifndef LOCAL
  _ssl_context.set_verify_mode(boost::asio::ssl::verify_peer);
  _ssl_context.set_default_verify_paths();
#endif
#endif
}

socket::~socket() {
#ifdef EMSCRIPTEN
  constexpr int code = 1000;
  constexpr const char* reason = "Client disconnecting";

  if (_socket) {
    emscripten_websocket_close(_socket, code, reason);
    emscripten_websocket_delete(_socket);
    _socket = 0;
  }
#else
  _ws.async_close(websocket::close_code::normal, [](beast::error_code ec) {
    std::println(stderr, "[socket] websocket close error: {}", ec.message());
  });
#endif
}

void socket::connect() {
#ifdef EMSCRIPTEN
  const std::string url =
      "https://socket." + std::string(emscripten_run_script_string("window.location.hostname"));

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
#else
  _resolver.async_resolve(
      "carimbo.run",
      "443",
      beast::bind_front_handler(&socket::on_resolve, this)
  );

  _thread = std::thread([this]() {
    _io_context.run();
  });

  _thread.detach();
#endif
}

void socket::emit(const std::string& topic, const std::string& data) {
  send(std::format(R"json({{"event": {{"topic": "{}", "data": {}}}}})json", topic, data));
}

void socket::on(const std::string& topic, std::function<void(const std::string& )>&& callback) {
  send(std::format(R"json({{"subscribe": "{}"}})json", topic));
  _callbacks[topic].emplace_back(std::move(callback));
}

void socket::rpc(const std::string& method, const std::string& arguments, std::function<void(const std::string& )>&& callback) {
  send(std::format(R"json({{"rpc": {{"request": {{"id": {}, "method": "{}", "arguments": "{}"}}}}}})json",
                   ++counter, method, arguments));
  _callbacks[method].emplace_back(std::move(callback));
}

#ifdef EMSCRIPTEN
void socket::handle_open(const EmscriptenWebSocketOpenEvent* event) {
  _connected = true;

  for (const auto& message : _queue) {
    send(message);
  }
  _queue.clear();
}

void socket::handle_message(const EmscriptenWebSocketMessageEvent* event) {
  if (!event->isText) {
    return;
  }

  std::string buffer(reinterpret_cast<const char*>(event->data), event->numBytes);

  on_message(buffer);
}

void socket::handle_error(const EmscriptenWebSocketErrorEvent* event) {
  invoke("error", "WebSocket error occurred");
}

void socket::handle_close(const EmscriptenWebSocketCloseEvent* event) {
}
#endif

#ifndef EMSCRIPTEN
void socket::on_resolve(beast::error_code ec, tcp::resolver::results_type results) {
  if (ec) {
    if (ec == boost::asio::error::operation_aborted)
      return;

    std::println(stderr, "[error] resolve error: {}", ec.message());
    return;
  }

  beast::get_lowest_layer(_ws).expires_after(std::chrono::seconds(30));
  beast::get_lowest_layer(_ws).async_connect(
      results,
      beast::bind_front_handler(
          &socket::on_connect,
          this
      )
  );
}

void socket::on_connect(beast::error_code ec, const tcp::resolver::results_type::endpoint_type& endpoint) {
  if (ec) {
    if (ec == boost::asio::error::operation_aborted)
      return;

    std::println(stderr, "[socket] connect error: {}", ec.message());
    return;
  }

  _connected = true;
  beast::get_lowest_layer(_ws).expires_after(std::chrono::seconds(30));

  SSL_set_tlsext_host_name(_ws.next_layer().native_handle(), "carimbo.run");

  _ws.next_layer().async_handshake(
      ssl::stream_base::client,
      beast::bind_front_handler(
          &socket::on_ssl_handshake,
          this
      )
  );
}

void socket::on_ssl_handshake(beast::error_code ec) {
  if (ec) {
    if (ec == boost::asio::error::operation_aborted)
      return;

    std::println(stderr, "[socket] ssl handshake error: {}", ec.message());
    return;
  }

  beast::get_lowest_layer(_ws).expires_never();
  _ws.set_option(websocket::stream_base::timeout::suggested(beast::role_type::client));

  _ws.async_handshake(
      "carimbo.run",
      "/socket",
      beast::bind_front_handler(&socket::on_handshake, this)
  );
}

void socket::on_handshake(beast::error_code ec) {
  if (ec) {
    if (ec == boost::asio::error::operation_aborted)
      return;

    std::println(stderr, "[socket] handshake error: {}", ec.message());
    return;
  }

  do_read();
}

void socket::on_read(beast::error_code ec, std::size_t bytes_transferred) {
  if (ec) {
    if (ec == boost::asio::error::operation_aborted)
      return;

    std::println(stderr, "[socket] read error: {}", ec.message());
    return;
  }

  std::string message = beast::buffers_to_string(_buffer.data());
  _buffer.consume(_buffer.size());
  on_message(message);

  do_read();
}

void socket::do_read() {
  _ws.async_read(
      _buffer,
      beast::bind_front_handler(
          &socket::on_read,
          this
      )
  );
}
#endif

void socket::on_message(const std::string& buffer) {
  const auto& j = json::parse(buffer, nullptr, false);

  if (j.value("command", "") == "ping") {
    send(R"json({"command": "pong"})json");
    return;
  }

  if (j.value("command", "") == "reload") {
#ifdef EMSCRIPTEN
    emscripten_run_script_string("window.location.reload()");
#endif
    return;
  }

  if (const auto& event = j.value("event", json::object()); !event.empty()) {
    invoke(
        event.at("topic").get_ref<const std::string& >(),
        event.at("data").dump()
    );

    return;
  }

  if (const auto& rpc = j.value("rpc", json::object()); !rpc.empty() && rpc.contains("response")) {
    const auto& response = rpc.at("response");
    if (response.contains("result")) [[likely]] {
      invoke(
          std::to_string(response.at("id").get<uint64_t>()),
          response.at("result").dump()
      );
    }

    if (response.contains("error")) [[unlikely]] {
      // TODO handle error
    }

    return;
  }
}

void socket::send(const std::string& message) {
  if (!_connected) {
    _queue.emplace_back(message);
    return;
  }

#ifdef EMSCRIPTEN
  emscripten_websocket_send_utf8_text(_socket, message.c_str());
#else
  net::post(_ws.get_executor(), [self = this, message]() {
    self->_ws.async_write(
        net::buffer(message),
        [](boost::system::error_code ec, std::size_t bytes_transferred) {
          if (ec) [[unlikely]] {
            std::println(stderr, "[socket] write error: {}", ec.message());
            return;
          }
        }
    );
  });
#endif
}

void socket::invoke(const std::string& event, const std::string& data) const {
  if (const auto it = _callbacks.find(event); it != _callbacks.end()) {
    const auto& callbacks = it->second;

    for (const auto& callback : callbacks) {
      callback(data);
    }
  }
}
