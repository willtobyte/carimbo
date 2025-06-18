#include "delay.hpp"

namespace framework {
void delay(uint32_t interval) noexcept {
  SDL_Delay(interval);
}
}
