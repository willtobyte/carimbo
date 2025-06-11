#include "timermanager.hpp"

using namespace framework;

uint32_t generic_wrapper(void* userdata, SDL_TimerID id, uint32_t interval, bool repeat) {
  UNUSED(id);

  auto* fn = static_cast<std::function<void()>*>(userdata);

  SDL_Event event{};
  event.type = static_cast<uint32_t>(input::event::type::timer);
  event.user.data1 = static_cast<void*>(fn);
  event.user.data2 = static_cast<void*>(new bool(repeat));

  SDL_PushEvent(&event);

  return repeat ? interval : 0;
}

uint32_t wrapper(void *userdata, SDL_TimerID id, uint32_t interval) {
  return generic_wrapper(userdata, id, interval, true);
}

uint32_t singleshot_wrapper(void *userdata, SDL_TimerID id, uint32_t interval) {
  return generic_wrapper(userdata, id, interval, false);
}

uint32_t timermanager::set(uint32_t interval, std::function<void()> fn) {
  return add_timer(interval, std::move(fn), true);
}

uint32_t timermanager::singleshot(uint32_t interval, std::function<void()> fn) {
  return add_timer(interval, std::move(fn), false);
}

void timermanager::clear(uint32_t id) {
  SDL_RemoveTimer(id);
}

uint32_t timermanager::add_timer(uint32_t interval, std::function<void()> fn, bool repeat) {
  auto* ptr = new std::function<void()>(std::move(fn));
  const auto id = SDL_AddTimer(interval, repeat ? wrapper : singleshot_wrapper, ptr);
  if (!id) [[unlikely]] {
    delete ptr;
    throw std::runtime_error(fmt::format("[SDL_AddTimer] {}", SDL_GetError()));
  }

  return id;
}
