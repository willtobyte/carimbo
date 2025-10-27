#pragma once

#define WEBSOCKET

#ifdef EMSCRIPTEN
 #include <emscripten.h>
 #ifdef WEBSOCKET
  #include <emscripten/websocket.h>
 #endif
#endif

#ifdef DEBUG
  #define SOL_ALL_SAFETIES_ON 1
  #define SOL_EXCEPTIONS_SAFE_PROPAGATION 1
#endif

#ifdef HAVE_SENTRY
 #include <sentry.h>
#endif

extern "C" {
  #include <lua.h>
  #include <lauxlib.h>
  #include <lualib.h>
}

#include <AL/al.h>
#include <AL/alc.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL.h>
#include <nlohmann/json.hpp>
#include <ogg/ogg.h>
#include <physfs.h>
#include <sol/sol.hpp>
#include <spng.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

#ifndef EMSCRIPTEN
 #include <boost/asio.hpp>
 #include <boost/asio/connect.hpp>
 #include <boost/asio/ssl.hpp>
 #include <boost/asio/strand.hpp>
 #include <boost/beast/core.hpp>
 #include <boost/beast/websocket.hpp>
 #include <boost/beast/websocket/ssl.hpp>
 #include <boost/config.hpp>
#endif

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <boost/functional/hash.hpp>

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <csignal>
#include <execution>
#include <filesystem>
#include <format>
#include <fstream>
#include <functional>
#include <iterator>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <numbers>
#include <optional>
#include <print>
#include <random>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <type_traits>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

#include "constant.hpp"
#include "defer.hpp"
#include "helper.hpp"
