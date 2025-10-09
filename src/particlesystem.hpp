#pragma once

#include "common.hpp"

namespace graphics {
struct alignas(16) particle final {
  float x, y;
  float vx, vy;
  float gx, gy;
  float av, af;
  float life, scale, angle;
  float alpha;
};

struct particleconf final {
  float x, y;
  bool active;
  std::shared_ptr<pixmap> pixmap;
  std::minstd_rand rng{std::random_device{}()};
  std::uniform_real_distribution<float> xspawnd;
  std::uniform_real_distribution<float> yspawnd;
  std::uniform_real_distribution<float> radiusd;
  std::uniform_real_distribution<double> angled;
  std::uniform_real_distribution<float> xveld;
  std::uniform_real_distribution<float> yveld;
  std::uniform_real_distribution<float> gxd;
  std::uniform_real_distribution<float> gyd;
  std::uniform_real_distribution<float> scaled;
  std::uniform_real_distribution<float> lifed;
  std::uniform_int_distribution<unsigned int> alphad;
  std::uniform_real_distribution<float> rotforced;
  std::uniform_real_distribution<float> rotveld;

  auto randradius() noexcept { return radiusd(rng); }
  auto randangle() noexcept { return angled(rng); }
  auto randxspawn() noexcept { return xspawnd(rng); }
  auto randyspawn() noexcept { return yspawnd(rng); }
  auto randxvel() noexcept { return xveld(rng); }
  auto randyvel() noexcept { return yveld(rng); }
  auto randgx() noexcept { return gxd(rng); }
  auto randgy() noexcept { return gyd(rng); }
  auto randscale() noexcept { return scaled(rng); }
  auto randlife() noexcept { return lifed(rng); }
  auto randalpha() noexcept { return static_cast<uint8_t>(alphad(rng)); }
  auto randrotforce() noexcept { return rotforced(rng); }
  auto randrotvel() noexcept { return rotveld(rng); }

  void set_active(bool v) noexcept { active = v; }
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

    void update(float delta) noexcept;

    void draw() const noexcept;

    std::shared_ptr<particlefactory> factory() const noexcept;

  private:
    std::shared_ptr<particlefactory> _factory;
    std::vector<std::shared_ptr<particlebatch>> _batches;
};
}
