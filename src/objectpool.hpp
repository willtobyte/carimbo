#pragma once

#include "common.hpp"

#include "singleton.hpp"
#include "envelope.hpp"

namespace framework {
class envelopepool_impl final {
private:
  struct pmr_deleter final {
    envelopepool_impl* pool;

    void operator()(envelope* ptr) const noexcept {
      if (pool && ptr) {
        pool->release_internal(ptr);
      }
    }
  };

public:
  using envelope_ptr = std::unique_ptr<envelope, pmr_deleter>;

private:
  std::pmr::unsynchronized_pool_resource _pool{{.max_blocks_per_chunk = 256, .largest_required_pool_block = sizeof(envelope)}};
  std::pmr::vector<envelope*> _available{&_pool};

  void expand(size_t minimum) {
    const size_t current_size = _available.size();
    size_t target = std::max(current_size, size_t(1));

    while (target < minimum) {
      target <<= 1;
    }

    _available.reserve(target);

    for (size_t i = current_size; i < target; ++i) {
      void* mem = _pool.allocate(sizeof(envelope), alignof(envelope));
      envelope* ptr = new (mem) envelope(&_pool);
      _available.push_back(ptr);
    }

    std::println("[pool<envelope>] expanded to {} objects", target);
  }

  void release_internal(envelope* ptr) noexcept {
    if (!ptr) {
      return;
    }

    ptr->reset();
    _available.push_back(ptr);
  }

public:
  envelopepool_impl() {
    expand(256);
  }

  ~envelopepool_impl() {
    for (envelope* ptr : _available) {
      if (ptr) {
        ptr->~envelope();
        _pool.deallocate(ptr, sizeof(envelope), alignof(envelope));
      }
    }
  }

  template<typename... Args>
  envelope_ptr acquire(Args&&... args) {
    if (_available.empty()) {
      const size_t current_capacity = _available.capacity();
      const size_t need = current_capacity ? current_capacity : 256;
      expand(need);
    }

    envelope* ptr = _available.back();
    _available.pop_back();

    if constexpr (sizeof...(Args) > 0) {
      ptr->reset(std::forward<Args>(args)...);
    }

    return envelope_ptr(ptr, pmr_deleter{this});
  }

  void release(envelope_ptr&& object) noexcept {
    object.reset();
  }

  void reserve(size_t count) {
    if (count > _available.size()) {
      expand(count);
    }
  }

  size_t size() const noexcept {
    return _available.size();
  }
};

template<typename T, auto& Name>
using uniquepool = envelopepool_impl;

inline constexpr char envelope_pool_name[] = "envelope";

using envelopepool = singleton<envelopepool_impl>;
}
