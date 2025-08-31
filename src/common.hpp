#pragma once

#define WEBSOCKET

#ifdef EMSCRIPTEN
 #include <emscripten.h>
 #ifdef WEBSOCKET
  #include <emscripten/websocket.h>
 #endif
#endif

#ifdef HAVE_BOOST
 #include <boost/asio.hpp>
 #include <boost/asio/connect.hpp>
 #include <boost/asio/ssl.hpp>
 #include <boost/asio/strand.hpp>
 #include <boost/beast/core.hpp>
 #include <boost/beast/websocket.hpp>
 #include <boost/beast/websocket/ssl.hpp>
 #include <boost/config.hpp>
#endif

#ifdef DEBUG
  #define SOL_ALL_SAFETIES_ON 1
  #define SOL_EXCEPTIONS_SAFE_PROPAGATION 1

  #if HAVE_BOOST
    #define BOOST_STACKTRACE_GNU_SOURCE_NOT_REQUIRED
    #include <boost/stacktrace.hpp>
    #define HAVE_STACKSTRACE
  #endif
#endif

extern "C" {
  #include <lua.h>
  #include <lauxlib.h>
  #include <lualib.h>
}

#include <AL/al.h>
#include <AL/alc.h>
#include <SDL3/SDL.h>
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
#include <csignal>
#include <print>
#include <typeinfo>
#include <format>
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
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

struct SDL_Deleter {
  template <typename T>
  void operator()(T* ptr) const {
    if (!ptr) return;

    if constexpr (requires { SDL_DestroyCond(ptr); }) SDL_DestroyCond(ptr);
    if constexpr (requires { SDL_FreeCursor(ptr); }) SDL_FreeCursor(ptr);
    if constexpr (requires { SDL_GL_DeleteContext(ptr); }) SDL_GL_DeleteContext(ptr);
    if constexpr (requires { SDL_CloseGamepad(ptr); }) SDL_CloseGamepad(ptr);
    if constexpr (requires { SDL_HapticClose(ptr); }) SDL_HapticClose(ptr);
    if constexpr (requires { SDL_JoystickClose(ptr); }) SDL_JoystickClose(ptr);
    if constexpr (requires { SDL_DestroyMutex(ptr); }) SDL_DestroyMutex(ptr);
    if constexpr (requires { SDL_FreePalette(ptr); }) SDL_FreePalette(ptr);
    if constexpr (requires { SDL_FreeFormat(ptr); }) SDL_FreeFormat(ptr);
    if constexpr (requires { SDL_DestroyRenderer(ptr); }) SDL_DestroyRenderer(ptr);
    if constexpr (requires { SDL_RWclose(ptr); }) SDL_RWclose(ptr);
    if constexpr (requires { SDL_DestroySemaphore(ptr); }) SDL_DestroySemaphore(ptr);
    if constexpr (requires { SDL_DestroySurface(ptr); }) SDL_DestroySurface(ptr);
    if constexpr (requires { SDL_DestroyTexture(ptr); }) SDL_DestroyTexture(ptr);
    if constexpr (requires { SDL_DestroyWindow(ptr); }) SDL_DestroyWindow(ptr);
  }
};

template <typename... Ts>
constexpr void UNUSED(const Ts&...) {}

constexpr long MINIMAL_USE_COUNT = 1;

static constexpr auto DELAY_MS = 1000 / 60;
