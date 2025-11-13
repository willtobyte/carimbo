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
      if (pool && ptr) pool->recycle(ptr);
    }
  };

public:
  using envelope_ptr = std::unique_ptr<envelope, pmr_deleter>;

private:
  std::pmr::unsynchronized_pool_resource _pool{{.max_blocks_per_chunk = 64, .largest_required_pool_block = sizeof(envelope)}};
  std::pmr::vector<envelope*> _available{&_pool};

  void expand(size_t count) {
    _available.reserve(_available.size() + count);
    for (size_t i = 0; i < count; ++i) {
      void* mem = _pool.allocate(sizeof(envelope), alignof(envelope));
      _available.push_back(std::construct_at(static_cast<envelope*>(mem), &_pool));
    }
  }

  void recycle(envelope* ptr) noexcept {
    if (ptr) {
      ptr->reset();
      _available.push_back(ptr);
    }
  }

public:
  envelopepool_impl() {
    expand(32);
  }

  ~envelopepool_impl() {
    for (envelope* ptr : _available) {
      if (ptr) {
        std::destroy_at(ptr);
        _pool.deallocate(ptr, sizeof(envelope), alignof(envelope));
      }
    }
  }

  template<typename... Args>
  envelope_ptr acquire(Args&&... args) {
    if (_available.empty()) {
      expand(_available.capacity() ? _available.capacity() : 32);
    }

    envelope* ptr = _available.back();
    _available.pop_back();

    if constexpr (sizeof...(Args) > 0) {
      ptr->reset(std::forward<Args>(args)...);
    }

    return envelope_ptr(ptr, pmr_deleter{this});
  }

  void release(envelope_ptr&& ptr) noexcept {
    ptr.reset();
  }

  void reserve(size_t count) {
    if (count > _available.size()) {
      expand(count - _available.size());
    }
  }

  size_t available() const noexcept {
    return _available.size();
  }
};

using envelopepool = singleton<envelopepool_impl>;

}