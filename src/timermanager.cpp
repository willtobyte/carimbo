#include "timermanager.hpp"

using namespace framework;

static uint32_t generic_wrapper(void* userdata, SDL_TimerID id, uint32_t interval, bool repeat) noexcept {
  UNUSED(id);

  SDL_Event event{};
  event.type = static_cast<uint32_t>(input::event::type::timer);
  event.user.data1 = userdata;
  SDL_PushEvent(&event);

  return repeat ? interval : 0;
}

uint32_t wrapper(void* userdata, SDL_TimerID id, uint32_t interval) noexcept {
  return generic_wrapper(userdata, id, interval, true);
}

uint32_t singleshot_wrapper(void* userdata, SDL_TimerID id, uint32_t interval) noexcept {
  return generic_wrapper(userdata, id, interval, false);
}

timermanager::timermanager() noexcept
    : _envelopepool(envelopepool::instance()) {
}

uint32_t timermanager::set(uint32_t interval, std::function<void()>&& fn) noexcept {
  return add_timer(interval, std::move(fn), true);
}

uint32_t timermanager::singleshot(uint32_t interval, std::function<void()>&& fn) noexcept {
  return add_timer(interval, std::move(fn), false);
}

void timermanager::clear(uint32_t id) noexcept {
  SDL_RemoveTimer(id);
  const auto it = _envelopemapping.find(id);
  if (it == _envelopemapping.end()) [[unlikely]] {
    return;
  }

  _envelopepool->release(std::unique_ptr<envelope>(it->second));
  _envelopemapping.erase(it);
}

void timermanager::purge() noexcept {
  for (auto& [id, ptr] : _envelopemapping) {
    SDL_RemoveTimer(id);
    _envelopepool->release(std::unique_ptr<envelope>(ptr));
  }

  _envelopemapping.clear();
}

uint32_t timermanager::add_timer(uint32_t interval, std::function<void()>&& fn, bool repeat) {
  const auto ptr = _envelopepool->acquire(timerenvelope(repeat, std::move(fn))).release();

  const auto id = SDL_AddTimer(interval, repeat ? wrapper : singleshot_wrapper, ptr);
  if (id) [[likely]] {
    _envelopemapping.emplace(id, ptr);
    return id;
  }

  _envelopepool->release(std::unique_ptr<envelope>(ptr));
  throw std::runtime_error(std::format("[SDL_AddTimer] {}", SDL_GetError()));
}
