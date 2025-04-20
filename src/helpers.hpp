#pragma once

#define panic(...) do { \
  SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal Error", fmt::format(__VA_ARGS__).c_str(), nullptr); \
  std::abort(); \
} while (0)

#ifndef EMSCRIPTEN
#ifdef WEBSOCKET
#include <boost/config.hpp>

namespace boost {
BOOST_NORETURN inline void throw_exception(std::exception const& e) {
  panic(e.what());
}

BOOST_NORETURN inline void throw_exception(std::exception const& e, boost::source_location const&) {
  panic(e.what());
}
}
#endif
#endif

template <typename... Ts>
constexpr void UNUSED(const Ts &...) {}

constexpr long MINIMAL_USE_COUNT = 1;

namespace geometry {
class size;
}

std::pair<std::vector<uint8_t>, geometry::size> _load_png(const std::string &filename);
