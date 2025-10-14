#pragma once

#include "common.hpp"

namespace framework {
class world final {
  public:
    world() noexcept;
    ~world() noexcept = default;

    void add(const std::shared_ptr<object>& object);
    void remove(const std::shared_ptr<object>& object);

    // void set_camera(std::shared_ptr<camera> camera) noexcept;

    void update(float delta) noexcept;
    void draw() const noexcept;

  private:
    std::unordered_map<uint64_t, std::weak_ptr<object>> _index;

    std::shared_ptr<uniquepool<envelope, framework::envelope_pool_name>> _envelopepool = envelopepool::instance();
};
}
