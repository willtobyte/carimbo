#pragma once

#ifdef _WIN32
#define NOMINMAX
#endif

#define SOL_NO_EXCEPTIONS 1

#define JSON_NOEXCEPTION 1

#ifdef DEBUG
#define SOL_ALL_SAFETIES_ON 1
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

template <typename... Args>
[[noreturn]] inline void panic(std::string_view format, Args&&... args) noexcept {
  const auto message = fmt::format(fmt::runtime(format), std::forward<Args>(args)...);

  fmt::println(stderr, "{}", message);
  SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal error", message.c_str(), nullptr);
  std::abort();
}

[[noreturn]] inline void panic(const std::exception& e) noexcept {
  panic("{}", e.what());
}

[[noreturn]] inline void panic(const sol::optional<std::string>& maybe_msg) noexcept {
  if (maybe_msg) {
    panic(
      "Lua is in a panic state and will now abort() the application\n"
      "error message: {}",
      *maybe_msg
    );
  } else {
    panic("Lua is in a panic state and will now abort() the application");
  }
}

#ifdef BOOST_NO_EXCEPTIONS
namespace boost {
BOOST_NORETURN inline void throw_exception(std::exception const& e) {
  panic(e);
}

BOOST_NORETURN inline void throw_exception(std::exception const& e, boost::source_location const&) {
  panic(e);
}
}
#endif

#define JSON_TRY_USER if(true)
#define JSON_CATCH_USER(exception) if(false)
#define JSON_THROW_USER(exception) panic(exception)

#include <nlohmann/json.hpp>
