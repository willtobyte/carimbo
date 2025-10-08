#pragma once

#include "common.hpp"

namespace graphics {
struct particle final {
  float x, y;
  float vx, vy;
  float gx, gy;
  float av, af;
  float life;
  float scale;
  float angle;
  uint32_t pixmap;
  uint8_t alpha;
};

struct particleconf final {
  float x, y;
  bool active;
  std::shared_ptr<pixmap> pixmap;
  std::mt19937 rng{std::random_device{}()};
  std::uniform_real_distribution<float> radiusdist;
  std::uniform_real_distribution<double> angledist;
  std::uniform_real_distribution<float> xstartdist;
  std::uniform_real_distribution<float> ystartdist;
  std::uniform_real_distribution<float> xveldist;
  std::uniform_real_distribution<float> yveldist;
  std::uniform_real_distribution<float> gxdist;
  std::uniform_real_distribution<float> gydist;
  std::uniform_real_distribution<float> scaledist;
  std::uniform_real_distribution<float> lifedist;
  std::uniform_int_distribution<unsigned int> alphadist;
  std::uniform_real_distribution<float> rotforcedist;
  std::uniform_real_distribution<float> rotveldist;

  auto randradius() noexcept { return radiusdist(rng); }
  auto randangle() noexcept { return angledist(rng); }
  auto randxstart() noexcept { return xstartdist(rng); }
  auto randystart() noexcept { return ystartdist(rng); }
  auto randxvel() noexcept { return xveldist(rng); }
  auto randyvel() noexcept { return yveldist(rng); }
  auto randgx() noexcept { return gxdist(rng); }
  auto randgy() noexcept { return gydist(rng); }
  auto randscale() noexcept { return scaledist(rng); }
  auto randlife() noexcept { return lifedist(rng); }
  auto randalpha() noexcept { return static_cast<uint8_t>(alphadist(rng)); }
  auto randrotforce() noexcept { return rotforcedist(rng); }
  auto randrotvel() noexcept { return rotveldist(rng); }

  void set_active(bool value) noexcept { active = value; }
};

struct particlebatch final {
  std::shared_ptr<particleconf> conf;
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

    void add(const std::shared_ptr<particlebatch>& batch) noexcept;

    void set(const std::vector<std::shared_ptr<particlebatch>>& batches) noexcept;

    void clear() noexcept;

    void update(float_t delta) noexcept;

    void draw() const noexcept;

    std::shared_ptr<particlefactory> factory() const noexcept;

  private:
    std::shared_ptr<particlefactory> _factory;
    std::vector<std::shared_ptr<particlebatch>> _batches;
};
}
