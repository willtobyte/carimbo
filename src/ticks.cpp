#include "ticks.hpp"

uint64_t ticks() noexcept { return SDL_GetTicks(); }
