#pragma once

#include "common.hpp"
#include "random.hpp"

struct particleprops final {
  float x, y;
  float hw, hh;
  bool active;
  bool spawning;
  std::shared_ptr<pixmap> pixmap;
  rng::xorshift128plus rng{std::random_device{}()};
  rng::uniform_real<float> xspawnd, yspawnd, radiusd, angled;
  rng::uniform_real<float> xveld, yveld, gxd, gyd;
  rng::uniform_real<float> scaled, lifed, rotforced, rotveld;
  rng::uniform_int<unsigned int> alphad;

  float randradius() noexcept { return radiusd(rng); }
  float randangle() noexcept { return angled(rng); }
  float randxspawn() noexcept { return xspawnd(rng); }
  float randyspawn() noexcept { return yspawnd(rng); }
  float randxvel() noexcept { return xveld(rng); }
  float randyvel() noexcept { return yveld(rng); }
  float randgx() noexcept { return gxd(rng); }
  float randgy() noexcept { return gyd(rng); }
  float randscale() noexcept { return scaled(rng); }
  float randlife() noexcept { return lifed(rng); }
  uint8_t randalpha() noexcept { return static_cast<uint8_t>(alphad(rng)); }
  float randrotforce() noexcept { return rotforced(rng); }
  float randrotvel() noexcept { return rotveld(rng); }

  void set_position(float xv, float yv) noexcept { x = xv; y = yv; }
};

struct particle final {
  std::vector<float> x, y, vx, vy, gx, gy;
  std::vector<float> life, scale, angle, av, af;
  std::vector<uint8_t> alpha;
  size_t count{0};

  void resize(size_t n) {
    count = n;
    x.resize(n); y.resize(n);
    vx.resize(n); vy.resize(n);
    gx.resize(n); gy.resize(n);
    life.resize(n); scale.resize(n);
    angle.resize(n); av.resize(n); af.resize(n);
    alpha.resize(n);
  }
};

struct particlebatch final {
  std::shared_ptr<particleprops> props;
  particle particles;
  std::vector<int> indices;
  std::vector<SDL_Vertex> vertices;
  std::vector<size_t> respawn;

  size_t size() const noexcept { return particles.count; }
};

class particlefactory final {
public:
  explicit particlefactory(std::shared_ptr<resourcemanager> resourcemanager);

  std::shared_ptr<particlebatch> create(std::string_view kind, float x, float y, bool spawning = true) const;

private:
  std::shared_ptr<resourcemanager> _resourcemanager;
};

class particlesystem final {
public:
  explicit particlesystem(std::shared_ptr<resourcemanager> resourcemanager);
  ~particlesystem() = default;

  void add(const std::shared_ptr<particlebatch>& batch);

  void set(std::vector<std::shared_ptr<particlebatch>> batches);

  void clear();

  void update(float delta);

  void draw() const;

  std::shared_ptr<particlefactory> factory() const noexcept;

private:
  std::shared_ptr<renderer> _renderer;
  std::shared_ptr<particlefactory> _factory;
  std::vector<std::shared_ptr<particlebatch>> _batches;
};
