#pragma once

template <typename... Ts>
constexpr void UNUSED(const Ts &...) {}

constexpr long MINIMAL_USE_COUNT = 1;
