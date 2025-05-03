#include "moment.hpp"

using namespace framework;

namespace framework {
uint64_t moment() { return SDL_GetTicks(); }
}
