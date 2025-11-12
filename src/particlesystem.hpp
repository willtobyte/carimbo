#pragma once

#include "common.hpp"

namespace graphics {
struct particleprops final {
  float x, y;
  bool active;
  bool emitting;
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
  std::uniform_real_distribution<double> rotforced;
  std::uniform_real_distribution<double> rotveld;

  auto randradius() { return radiusd(rng); }
  auto randangle() { return angled(rng); }
  auto randxspawn() { return xspawnd(rng); }
  auto randyspawn() { return yspawnd(rng); }
  auto randxvel() { return xveld(rng); }
  auto randyvel() { return yveld(rng); }
  auto randgx() { return gxd(rng); }
  auto randgy() { return gyd(rng); }
  auto randscale() { return scaled(rng); }
  auto randlife() { return lifed(rng); }
  auto randalpha() { return static_cast<uint8_t>(alphad(rng)); }
  auto randrotforce() { return rotforced(rng); }
  auto randrotvel() { return rotveld(rng); }

  void set_placement(float xv, float yv) { x = xv; y = yv; }
};

struct alignas(64) particle final {
  float x, y;
  float vx, vy;
  float gx, gy;
  float life;
  float scale;
  double angle;
  double av, af;
  uint8_t alpha;
  uint8_t _padding[7];
};

struct particlebatch final {
  std::shared_ptr<particleprops> props;
  std::vector<particle> particles;

  std::size_t size() const {
    return particles.size();
  }
};

class particlefactory final {
  public:
    explicit particlefactory(std::shared_ptr<framework::resourcemanager> resourcemanager);

    std::shared_ptr<particlebatch> create(std::string_view kind, float x, float y, bool emitting = true) const;

  private:
    std::shared_ptr<framework::resourcemanager> _resourcemanager;
};

class particlesystem final {
  public:
    explicit particlesystem(std::shared_ptr<framework::resourcemanager> resourcemanager);
    ~particlesystem() = default;

    void add(const std::shared_ptr<particlebatch>& batch);

    void set(const std::vector<std::shared_ptr<particlebatch>>& batches);

    void clear();

    void update(float delta);

    void draw() const;

    std::shared_ptr<particlefactory> factory() const;

  private:
    std::shared_ptr<particlefactory> _factory;
    std::vector<std::shared_ptr<particlebatch>> _batches;
};
}