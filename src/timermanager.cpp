#include "timermanager.hpp"

#include "envelope.hpp"
#include "event.hpp"
#include "objectpool.hpp"

constexpr auto event_type = static_cast<uint32_t>(event::type::timer);

struct context final {
  envelopepool_impl* pool;
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

  return false;
}

static uint32_t generic_wrapper(void* userdata, uint32_t id, uint32_t interval, bool repeat) {
  SDL_Event event{};
  event.type = event_type;
  event.user.data1 = userdata;
  SDL_PushEvent(&event);

  return repeat ? interval : 0;
}

uint32_t wrapper(void* userdata, uint32_t id, uint32_t interval) {
  return generic_wrapper(userdata, id, interval, true);
}

uint32_t singleshot_wrapper(void* userdata, uint32_t id, uint32_t interval) {
  return generic_wrapper(userdata, id, interval, false);
}

timermanager::timermanager()
    : _envelopepool(envelopepool::instance()) {
  _envelopemapping.reserve(16);
}

timermanager::~timermanager() noexcept {
  clear();
}

uint32_t timermanager::set(uint32_t interval, sol::protected_function fn) {
  return add_timer(interval, functor{std::move(fn)}, true);
}

uint32_t timermanager::singleshot(uint32_t interval, sol::protected_function fn) {
  return add_timer(interval, functor{std::move(fn)}, false);
}

void timermanager::cancel(uint32_t id) noexcept {
  const auto it = _envelopemapping.find(id);
  if (it == _envelopemapping.end()) [[unlikely]] {
    return;
  }

  auto* ptr = it->second;

  SDL_RemoveTimer(id);
  _envelopemapping.erase(it);

  if (ptr) {
    if (auto* payload = std::get_if<timerenvelope>(&ptr->payload)) {
      payload->fn = nullptr;
    }

    context ctx{&_envelopepool, ptr};
    SDL_FilterEvents(filter, &ctx);
    _envelopepool.release(ptr);
  }
}

void timermanager::clear() noexcept {
  while (!_envelopemapping.empty()) {
    auto it = _envelopemapping.begin();
    const auto id = it->first;

    cancel(id);
  }
}

uint32_t timermanager::add_timer(uint32_t interval, functor&& fn, bool repeat) noexcept {
  auto envelope = _envelopepool.acquire();
  envelope->reset(repeat, std::move(fn));
  auto* ptr = envelope.release();

  const auto id = SDL_AddTimer(interval, repeat ? wrapper : singleshot_wrapper, ptr);
  assert(id != 0 && std::format("[SDL_AddTimer] {}", SDL_GetError()).c_str());

  if (repeat) {
    _envelopemapping.emplace(id, ptr);
  }

  return id;
}
