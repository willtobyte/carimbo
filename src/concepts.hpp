#pragma once

#include <type_traits>

namespace geometry {
template<typename T>
concept numeric = std::is_arithmetic_v<T>;

template<typename T>
concept floating_point = std::is_floating_point_v<T>;

template<typename T>
concept integral = std::is_integral_v<T>;
}