#include "timermanager.hpp"

using namespace framework;

uint32_t generic_wrapper(void* userdata, SDL_TimerID id, uint32_t interval, bool repeat) {
  UNUSED(id);

  SDL_Event event{};
  event.type = static_cast<uint32_t>(input::event::type::timer);
  event.user.data1 = userdata;
  SDL_PushEvent(&event);

  return repeat ? interval : 0;
}

uint32_t wrapper(void *userdata, SDL_TimerID id, uint32_t interval) {
  return generic_wrapper(userdata, id, interval, true);
}

uint32_t singleshot_wrapper(void *userdata, SDL_TimerID id, uint32_t interval) {
  return generic_wrapper(userdata, id, interval, false);
}

timermanager::timermanager() noexcept
    : _envelopepool(envelopepool::instance()){
}

uint32_t timermanager::set(uint32_t interval, std::function<void()>&& fn) {
  return add_timer(interval, std::move(fn), true);
}

uint32_t timermanager::singleshot(uint32_t interval, std::function<void()>&& fn) {
  return add_timer(interval, std::move(fn), false);
}

void timermanager::clear(uint32_t id) {
  SDL_RemoveTimer(id);

  const auto it = _repeatmapping.find(id);
  if (it == _repeatmapping.end()) {
    return;
  }

  _envelopepool->release(std::unique_ptr<envelope>(it->second));
  _repeatmapping.erase(it);
}

uint32_t timermanager::add_timer(uint32_t interval, std::function<void()>&& fn, bool repeat) {
  auto ptr = _envelopepool->acquire(timerenvelope(repeat, std::move(fn))).release();

  const auto id = SDL_AddTimer(interval, repeat ? wrapper : singleshot_wrapper, ptr);
  if (id) [[likely]] {
    if (repeat) {
      _repeatmapping.emplace(id, ptr);
    }

    return id;
  }

  _envelopepool->release(std::unique_ptr<envelope>(ptr));
  throw std::runtime_error(std::format("[SDL_AddTimer] {}", SDL_GetError()));
}
