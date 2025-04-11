#pragma once

#ifdef _WIN32
#define NOMINMAX
#endif

#ifdef DEBUG
#define SOL_ALL_SAFETIES_ON 1
#define SOL_PRINT_ERRORS 1
#endif

#define SOL_LUAJIT 1

#define WEBSOCKET

#ifdef EMSCRIPTEN
#include <emscripten.h>
#ifdef WEBSOCKET
#include <emscripten/websocket.h>
#endif
#endif

#ifndef EMSCRIPTEN
#ifdef WEBSOCKET
#include <boost/asio.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#endif
#endif

#include <AL/al.h>
#include <AL/alc.h>
#include <SDL3/SDL.h>
#include <fmt/format.h>
#include <nlohmann/json.hpp>
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
