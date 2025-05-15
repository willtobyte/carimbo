#pragma once

#ifdef _WIN32
  #define NOMINMAX
#endif

#define SOL_ALL_SAFETIES_ON 1
#define SOL_EXCEPTIONS_SAFE_PROPAGATION 1
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

#ifdef HAVE_BOOST
  #include <boost/asio.hpp>
  #include <boost/asio/connect.hpp>
  #include <boost/asio/ssl.hpp>
  #include <boost/asio/strand.hpp>
  #include <boost/beast/core.hpp>
  #include <boost/beast/websocket.hpp>
  #include <boost/beast/websocket/ssl.hpp>
  #include <boost/config.hpp>

  #define BOOST_STACKTRACE_GNU_SOURCE_NOT_REQUIRED 1
  #include <boost/stacktrace.hpp>
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
#include <nlohmann/json.hpp>

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <csignal>
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
