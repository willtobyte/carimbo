#pragma once

#include "common.hpp"

namespace graphics {
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

  void set_placement(float xv, float yv) noexcept { x = xv; y = yv; }
};

struct particlebatch final {
  std::shared_ptr<particleconf> conf;

  std::vector<float> x, y;
  std::vector<float> vx, vy;
  std::vector<float> gx, gy;
  std::vector<float> av, af;
  std::vector<float> life;
  std::vector<float> scale;
  std::vector<float> angle;
  std::vector<std::uint8_t> alpha;

  std::size_t size() const noexcept {
    return x.size();
  }
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
