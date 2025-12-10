#pragma once

#include "common.hpp"

struct particleprops final {
  float x, y;
  float hw, hh;
  bool active;
  bool spawning;
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

  void set_position(float xv, float yv) noexcept { x = xv; y = yv; }
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
  std::vector<int> indices;
  std::vector<SDL_Vertex> vertices;

  std::size_t size() const noexcept {
    return particles.size();
  }
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