#include "timermanager.hpp"

using namespace framework;

constexpr auto event_type = static_cast<uint32_t>(input::event::type::timer);

struct context final {
  uniquepool<envelope, envelope_pool_name>* pool;
  envelope* target_ptr;
};

static bool filter(void* userdata, SDL_Event* e) {
  if (e->type != event_type) {
    return true;
  }

  if (!e->user.data1) {
    return false;
  }

  auto* ctx = static_cast<context*>(userdata);
  auto* ptr = static_cast<envelope*>(e->user.data1);

  if (ctx->target_ptr && ptr != ctx->target_ptr) {
    return true;
  }

  ctx->pool->release(std::unique_ptr<envelope>(ptr));

  return false;
}

static uint32_t generic_wrapper(void* userdata, SDL_TimerID id, uint32_t interval, bool repeat) {
  SDL_Event event{};
  event.type = event_type;
  event.user.data1 = userdata;
  SDL_PushEvent(&event);

  return repeat ? interval : 0;
}

uint32_t wrapper(void* userdata, SDL_TimerID id, uint32_t interval) {
  return generic_wrapper(userdata, id, interval, true);
}

uint32_t singleshot_wrapper(void* userdata, SDL_TimerID id, uint32_t interval) {
  return generic_wrapper(userdata, id, interval, false);
}

timermanager::timermanager()
    : _envelopepool(envelopepool::instance()) {
  _envelopemapping.reserve(16);
}

uint32_t timermanager::set(uint32_t interval, sol::protected_function fn) {
  return add_timer(interval, std::move(interop::wrap_fn(fn)), true);
}

uint32_t timermanager::singleshot(uint32_t interval, sol::protected_function fn) {
  return add_timer(interval, std::move(interop::wrap_fn(fn)), false);
}

void timermanager::cancel(uint32_t id) {
  SDL_RemoveTimer(id);
  const auto it = _envelopemapping.find(id);
  if (it == _envelopemapping.end()) [[unlikely]] {
    return;
  }

  auto* ptr = it->second;
  _envelopemapping.erase(it);

  if (ptr) {
    context ctx{_envelopepool.get(), ptr};
    SDL_FilterEvents(filter, &ctx);
  }
}

void timermanager::clear() {
  for (auto& [id, ptr] : _envelopemapping) {
    SDL_RemoveTimer(id);
  }

  context ctx{_envelopepool.get(), nullptr};
  SDL_FilterEvents(filter, &ctx);

  for (auto& [id, ptr] : _envelopemapping) {
    if (ptr) {
      _envelopepool->release(std::unique_ptr<envelope>(ptr));
    }
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
