#pragma once

#ifdef _WIN32
#define NOMINMAX
#endif

#define SOL_NO_EXCEPTIONS 1
#define JSON_NOEXCEPTION 1

#ifdef DEBUG
#define SOL_ALL_SAFETIES_ON 1
#define SOL_PRINT_ERRORS 1
#endif

#ifndef EMSCRIPTEN
#define SOL_LUAJIT 1
#endif

#define WEBSOCKET 1

#ifdef EMSCRIPTEN
#include <emscripten.h>
#ifdef WEBSOCKET
#include <emscripten/websocket.h>
#endif
#endif

#include <fmt/format.h>

#ifndef EMSCRIPTEN
#ifdef WEBSOCKET
#define BOOST_NO_EXCEPTIONS 1
#include <boost/asio.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/config.hpp>

namespace boost {
BOOST_NORETURN inline void throw_exception(std::exception const& e) {
  fmt::println(stderr, "{}", e.what());
  std::abort();
}

BOOST_NORETURN inline void throw_exception(std::exception const& e, boost::source_location const&) {
  fmt::println(stderr, "{}", e.what());
  std::abort();
}
}
#endif
#endif

#include <AL/al.h>
#include <AL/alc.h>
#include <SDL3/SDL.h>
#include <ogg/ogg.h>
#include <physfs.h>
#include <sol/sol.hpp>
#include <spng.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iterator>
#include <limits>
#include <unordered_set>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <random>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include "constants.hpp"
#include "deleters.hpp"
#include "helpers.hpp"

#define panic(...) do { \
  const auto e = fmt::format(__VA_ARGS__); \
  fmt::println(stderr, "{}", e); \
  SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal Error", e.c_str(), nullptr); \
  std::abort(); \
} while (0)

void sol_panic(const sol::optional<std::string> &maybe_message) {
  std::string_view message = maybe_message
    ? std::string_view{*maybe_message}
    : std::string_view{};

  panic(
    "Lua is in a panic state and will now abort() the application{}{}",
    message.empty() ? "" : "\nerror message: ",
    message
  );
}

#define JSON_TRY_USER if(true)
#define JSON_CATCH_USER(exception) if(false)
#define JSON_THROW_USER(exception) panic("JSON user error: {}", (exception).what())

#include <nlohmann/json.hpp>
