#pragma once

template <typename... Ts>
constexpr void UNUSED(const Ts &...) {}

constexpr long MINIMAL_USE_COUNT = 1;

namespace geometry {
class size;
}

std::pair<std::vector<uint8_t>, geometry::size> _load_png(const std::string &filename);
