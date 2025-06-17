#include "moment.hpp"

using namespace framework;

namespace framework {
uint64_t moment() noexcept { return SDL_GetTicks(); }
}
