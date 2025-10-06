#pragma once

#include "common.hpp"

namespace graphics {
struct particle final {
  double angle;
  float x, y;
  float vx, vy;
  float gx, gy;
  float life;
  uint32_t pixmap;
  uint8_t alpha;
};

struct emitter final {
  float x, y;
  uint32_t pixmap;
  std::mt19937 rng{std::random_device{}()};
  std::uniform_real_distribution<float> xveldist;
  std::uniform_real_distribution<float> yveldist;
  std::uniform_real_distribution<float> gxdist;
  std::uniform_real_distribution<float> gydist;
  std::uniform_real_distribution<float> lifedist;

  auto randxvel() noexcept { return xveldist(rng); }
  auto randyvel() noexcept { return yveldist(rng); }
  auto randgx() noexcept { return gxdist(rng); }
  auto randgy() noexcept { return gydist(rng); }
  auto randlife() noexcept { return lifedist(rng); }
};

class particlesystem final {
  public:
    explicit particlesystem(std::shared_ptr<framework::resourcemanager> resourcemanager) noexcept;
    ~particlesystem() noexcept = default;

    void create(const std::string& name, const std::string& kind, float_t x, float_t y);

    void destroy(const std::string& name) noexcept;

    void update(float_t delta) noexcept;

    void draw() const noexcept;

  private:
    std::atomic<uint32_t> _counter{0};
    std::shared_ptr<framework::resourcemanager> _resourcemanager;
    std::unordered_map<std::string, std::vector<particle>> _particles;
    std::unordered_map<uint32_t, std::shared_ptr<pixmap>> _pixmaps;
    std::unordered_map<std::string, emitter> _emitters;
};
}
