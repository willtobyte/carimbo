#pragma once

static constexpr auto MINIMAL_USE_COUNT = 1L;

static constexpr auto WORLD_SUBSTEPS = 4;

static constexpr float FIXED_TIMESTEP = 1.0f / 60.0f;

static constexpr auto DEGREES_TO_RADIANS = std::numbers::pi_v<float> / 180.0f;

static constexpr auto RADIANS_TO_DEGREES = 180.0f / std::numbers::pi_v<float>;

constexpr auto KILOBYTE = 1024uz;

constexpr auto READ_BUFFER_SIZE = 1 * KILOBYTE * KILOBYTE;

constexpr auto PHYSFS_BUFFER_SIZE = 4 * READ_BUFFER_SIZE;

static constexpr auto epsilon = std::numeric_limits<float>::epsilon();
