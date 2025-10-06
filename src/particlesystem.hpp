#pragma once

#include "common.hpp"

namespace graphics {
struct particle final {
  double angle;
  float x, y;
  float vx, vy;
  float gx, gy;
  float life;
  uint32_t frame;
  uint32_t pixmap;
  uint8_t alpha;
  uint8_t pad[3];
};

class particlesystem final {
  public:
    explicit particlesystem(std::shared_ptr<framework::resourcemanager> resourcemanager) noexcept;
    ~particlesystem() noexcept = default;

    void create(const std::string& name, const std::string& kind);

    void destroy(const std::string& name) noexcept;

    void update(float_t delta) noexcept;

    void draw() const noexcept;

  private:
    std::atomic<uint32_t> _counter{0};
    std::shared_ptr<framework::resourcemanager> _resourcemanager;
    std::unordered_map<std::string, std::vector<particle>> _particles;
    std::unordered_map<uint32_t, std::shared_ptr<pixmap>> _pixmaps;
};
}
