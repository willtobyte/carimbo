#pragma once

#include "common.hpp"

namespace graphics {
struct particle final {
  double_t angle;
  float x, y, life;
  uint32_t frame;
  uint32_t pixmap;
  uint8_t alpha;
};

class particlesystem final {
  public:
    particlesystem() noexcept = default;
    ~particlesystem() noexcept = default;

    void update() noexcept;

    void draw() const noexcept;

  private:
    std::unordered_map<std::string, std::vector<particle>> _particles;
    std::unordered_map<uint32_t, std::shared_ptr<pixmap>> _pixmaps;
};
}
