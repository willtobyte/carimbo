#pragma once

inline constexpr auto MINIMAL_USE_COUNT = 1L;

inline constexpr auto WORLD_SUBSTEPS = 4;

inline constexpr float FIXED_TIMESTEP = 1.0f / 60.0f;

inline constexpr auto FLUSH_INTERVAL = 10000u;

inline constexpr float DEGREES_TO_RADIANS = std::numbers::pi_v<float> / 180.0f;

inline constexpr float RADIANS_TO_DEGREES = 180.0f / std::numbers::pi_v<float>;

inline constexpr auto epsilon = std::numeric_limits<float>::epsilon();
