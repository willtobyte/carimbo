#pragma once

#include "common.hpp"

#include "singleton.hpp"
#include "envelope.hpp"

inline constexpr size_t PoolInitialCapacity = 512;

class envelopepool_impl final {
public:
  using envelope_ptr = std::unique_ptr<envelope, void(*)(envelope*)>;

private:
  std::pmr::monotonic_buffer_resource _upstream;
  std::pmr::unsynchronized_pool_resource _pool{&_upstream};
  std::pmr::vector<envelope*> _all{&_upstream};
  std::pmr::vector<envelope*> _free{&_upstream};

  void expand(size_t minimum = 0) {
    size_t target = _all.capacity();
    target = target == 0 ? 1 : target;
    target = boost::core::bit_ceil(std::max(target, minimum)) << 1;

    _all.reserve(target);
    _free.reserve(target);

    for (size_t i = _all.size(); i < target; ++i) {
      auto* ptr = std::construct_at(
        static_cast<envelope*>(_pool.allocate(sizeof(envelope), alignof(envelope))),
        &_pool
      );
      _all.emplace_back(ptr);
      _free.emplace_back(ptr);
    }

    std::println("[envelopepool] expanded to {} objects", target);
  }

  static void release_envelope(envelope* ptr) noexcept;

public:
  envelopepool_impl() { expand(PoolInitialCapacity); }
  ~envelopepool_impl() = default;

  envelopepool_impl(const envelopepool_impl&) = delete;
  envelopepool_impl& operator=(const envelopepool_impl&) = delete;

  template<typename... Args>
  [[nodiscard]] envelope_ptr acquire(Args&&... args) {
    if (_free.empty()) [[unlikely]] {
      expand(_all.capacity());
    }
    auto* ptr = _free.back();
    _free.pop_back();

    if constexpr (sizeof...(Args) > 0) {
      ptr->reset(std::forward<Args>(args)...);
    }

    return {ptr, release_envelope};
  }

  void release(envelope* ptr) noexcept {
    if (ptr) {
      ptr->reset();
      _free.emplace_back(ptr);
    }
  }

  [[nodiscard]] std::pmr::memory_resource* resource() noexcept {
    return &_pool;
  }
};

using envelopepool = singleton<envelopepool_impl>;

inline void envelopepool_impl::release_envelope(envelope* ptr) noexcept {
  if (ptr) {
    ptr->reset();
    envelopepool::instance()._free.emplace_back(ptr);
  }
}
