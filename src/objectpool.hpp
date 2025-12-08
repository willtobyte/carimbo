#pragma once

#include "common.hpp"

#include "singleton.hpp"
#include "envelope.hpp"


const constexpr auto InitialCapacity = 512uz;
const constexpr auto ChunkSize = 64uz;
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
  std::pmr::unsynchronized_pool_resource _pool{{.max_blocks_per_chunk = ChunkSize, .largest_required_pool_block = sizeof(envelope)}};
  std::pmr::vector<envelope*> _available{&_pool};

  void expand(size_t minimum) {
    size_t target = _available.capacity();
    if (target == 0) target = 1;

    target = boost::core::bit_ceil(std::max(target, minimum)) << 1;

    _available.reserve(target);

    for (auto count = target - _available.size(); count-- > 0uz;) {
      void* mem = _pool.allocate(sizeof(envelope), alignof(envelope));
      _available.emplace_back(std::construct_at(static_cast<envelope*>(mem), &_pool));
    }

    std::println("[pool<envelope>] expanded to {} objects", target);
  }

  void recycle(envelope* ptr) noexcept {
    ptr->reset();
    _available.emplace_back(ptr);
  }

public:
  envelopepool_impl() {
    expand(InitialCapacity);
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
  [[nodiscard]] envelope_ptr acquire(Args&&... args) {
    if (_available.empty()) {
      expand(_available.capacity());
    }

    envelope* ptr = _available.back();
    _available.pop_back();

    if constexpr (sizeof...(Args) > 0) {
      ptr->reset(std::forward<Args>(args)...);
    }

    return envelope_ptr(ptr, pmr_deleter{this});
  }

  void release(envelope* ptr) noexcept {
    if (ptr) {
      recycle(ptr);
    }
  }
};

using envelopepool = singleton<envelopepool_impl>;
