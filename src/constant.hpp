#pragma once

static constexpr auto MINIMAL_USE_COUNT = 1L;

static constexpr auto WORLD_SUBSTEPS = 4;

static constexpr auto FIXED_TIMESTEP = 1.0f / 60.0f;

static constexpr auto MAX_DELTA = 1.0f / 30.0f;

static constexpr auto DEGREES_TO_RADIANS = std::numbers::pi_v<float> / 180.0f;

static constexpr auto RADIANS_TO_DEGREES = 180.0f / std::numbers::pi_v<float>;

static constexpr auto epsilon = std::numeric_limits<float>::epsilon();
