#include "timermanager.hpp"

using namespace framework;

constexpr auto event_type = static_cast<uint32_t>(input::event::type::timer);

static uint32_t generic_wrapper(void* userdata, SDL_TimerID id, uint32_t interval, bool repeat) noexcept {
  SDL_Event event{};
  event.type = event_type;
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
  _envelopemapping.reserve(16);
}

uint32_t timermanager::set(uint32_t interval, std::function<void()>&& fn) noexcept {
  return add_timer(interval, std::move(fn), true);
}

uint32_t timermanager::singleshot(uint32_t interval, std::function<void()>&& fn) noexcept {
  return add_timer(interval, std::move(fn), false);
}

void timermanager::cancel(uint32_t id) noexcept {
  SDL_RemoveTimer(id);
  const auto it = _envelopemapping.find(id);
  if (it == _envelopemapping.end()) [[unlikely]] {
    return;
  }

  _envelopemapping.erase(it);
}

void timermanager::clear() noexcept {
  SDL_Event event;
  while (SDL_PeepEvents(&event, 1, SDL_GETEVENT, event_type, event_type) > 0) {
    if (!event.user.data1) {
      continue;
    }

    auto* ptr = static_cast<framework::envelope*>(event.user.data1);
    _envelopepool->release(std::unique_ptr<framework::envelope>(ptr));
  }

  for (auto& [id, ptr] : _envelopemapping) {
    SDL_RemoveTimer(id);
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
