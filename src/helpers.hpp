#pragma once

#include <boost/config.hpp>

#ifndef EMSCRIPTEN
#ifdef WEBSOCKET
namespace boost {
BOOST_NORETURN inline void throw_exception(std::exception const& e) {
  SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal Error", e.what(), nullptr);
  std::abort();
}

BOOST_NORETURN inline void throw_exception(std::exception const& e, boost::source_location const&) {
  SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal Error", e.what(), nullptr);
  std::abort();
}
}
#endif
#endif

#define panic(...) do { \
  SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal Error", fmt::format(__VA_ARGS__).c_str(), nullptr); \
  std::abort(); \
} while (0)

template <typename... Ts>
constexpr void UNUSED(const Ts &...) {}

constexpr long MINIMAL_USE_COUNT = 1;

namespace geometry {
class size;
}

std::pair<std::vector<uint8_t>, geometry::size> _load_png(const std::string &filename);
