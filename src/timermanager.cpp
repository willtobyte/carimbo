#include "timermanager.hpp"

using namespace framework;

uint32_t generic_wrapper(void *userdata, SDL_TimerID id, uint32_t interval, bool repeat) {
  UNUSED(id);
  const auto fn = static_cast<std::function<void()> *>(userdata);

#ifdef EMSCRIPTEN
  (*fn)();
  if (!repeat) {
    delete fn;
  }
#else
  SDL_Event event{};
  event.type = input::eventtype::timer;
  event.user.data1 = fn;
  event.user.data2 = new bool(repeat);

  SDL_PushEvent(&event);
#endif

  return repeat ? interval : 0;
}

uint32_t wrapper(void *userdata, SDL_TimerID id, uint32_t interval) {
  return generic_wrapper(userdata, id, interval, true);
}

uint32_t singleshot_wrapper(void *userdata, SDL_TimerID id, uint32_t interval) {
  return generic_wrapper(userdata, id, interval, false);
}

timermanager::~timermanager() noexcept {
  for (const auto &timer : _timers) {
    SDL_RemoveTimer(timer.first);
  }

  _timers.clear();
}

int32_t timermanager::set(int32_t interval, std::function<void()> fn) {
  return add_timer(interval, fn, true);
}

int32_t timermanager::singleshot(int32_t interval, std::function<void()> fn) {
  return add_timer(interval, fn, false);
}

void timermanager::clear(int32_t id) noexcept {
  if (auto it = _timers.find(id); it != _timers.end()) {
    SDL_RemoveTimer(id);
    _timers.erase(it);
  }
}

int32_t timermanager::add_timer(int32_t interval, std::function<void()> fn, bool repeat) {
  const auto ptr = new std::function<void()>(fn);
  const auto id = SDL_AddTimer(interval, repeat ? wrapper : singleshot_wrapper, ptr);
  if (id) [[likely]] {
    return id;
  }

  delete ptr;
  throw std::runtime_error(fmt::format("[SDL_AddTimer] failed to set timer. reason: {}", SDL_GetError()));
}
