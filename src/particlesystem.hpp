#pragma once

#include "common.hpp"

namespace graphics {
struct particle final {
  float x, y, life, angle;
  uint32_t frame;
  uint32_t pixmap;
  uint8_t alpha;
  uint8_t pad[3];
};

class particlesystem final {
  public:
    particlesystem() noexcept = default;
    ~particlesystem() noexcept = default;

    void update() noexcept;

    void draw() const noexcept;

  private:
    std::unordered_map<uint32_t, std::vector<particle>> _particles;
    std::unordered_map<uint32_t, std::shared_ptr<pixmap>> _pixmaps;
};
}
