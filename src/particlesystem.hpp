#pragma once

#include "common.hpp"

namespace graphics {
struct particle final {
  double angle;
  float x, y;
  float vx, vy;
  float gx, gy;
  float life;
  float scale;
  uint32_t pixmap;
  uint8_t alpha;
};

struct conf final {
  float x, y;
  std::shared_ptr<pixmap> pixmap;
  std::mt19937 rng{std::random_device{}()};
  std::uniform_real_distribution<float> xveldist;
  std::uniform_real_distribution<float> yveldist;
  std::uniform_real_distribution<float> gxdist;
  std::uniform_real_distribution<float> gydist;
  std::uniform_real_distribution<float> scaledist;
  std::uniform_real_distribution<float> lifedist;
  std::uniform_int_distribution<unsigned int> alphadist;

  auto randxvel() noexcept { return xveldist(rng); }
  auto randyvel() noexcept { return yveldist(rng); }
  auto randgx() noexcept { return gxdist(rng); }
  auto randgy() noexcept { return gydist(rng); }
  auto randscale() noexcept { return scaledist(rng); }
  auto randlife() noexcept { return lifedist(rng); }
  auto randalpha() noexcept { return static_cast<uint8_t>(alphadist(rng)); }
};

struct particlebatch final {
  conf conf;
  std::vector<particle> particles;
};

class particlefactory final {
  public:
    explicit particlefactory(std::shared_ptr<framework::resourcemanager> resourcemanager) noexcept;

    std::shared_ptr<particlebatch> create(const std::string& kind, float x, float y) const;

  private:
    std::shared_ptr<framework::resourcemanager> _resourcemanager;
};

class particlesystem final {
  public:
    explicit particlesystem(std::shared_ptr<framework::resourcemanager> resourcemanager) noexcept;
    ~particlesystem() noexcept = default;

    void add(std::shared_ptr<particlebatch> batch) noexcept;

    void set(std::vector<std::shared_ptr<particlebatch>> batches) noexcept;

    void clear() noexcept;

    void update(float_t delta) noexcept;

    void draw() const noexcept;

    std::shared_ptr<particlefactory> factory() const noexcept;

  private:
    std::shared_ptr<particlefactory> _factory;
    std::vector<std::shared_ptr<particlebatch>> _batches;
};
}
