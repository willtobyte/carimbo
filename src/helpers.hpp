#pragma once

#define panic(...) do { \
  const auto message = fmt::format(__VA_ARGS__); \
  fmt::println(stderr, "{}", message); \
  SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal Error", message.c_str(), nullptr); \
  std::abort(); \
} while (0)

#ifndef EMSCRIPTEN
#ifdef WEBSOCKET
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

template <typename... Ts>
constexpr void UNUSED(const Ts &...) {}

constexpr long MINIMAL_USE_COUNT = 1;
