#pragma once

#include "common.hpp"

#include "random.hpp"

struct cache final {
  size_t count;
  std::pair<float, float> xspawn, yspawn;
  std::pair<float, float> radius, angle;
  std::pair<float, float> scale, life;
  std::pair<float, float> xvel, yvel;
  std::pair<float, float> gx, gy;
  std::pair<float, float> rforce, rvel;
  std::shared_ptr<pixmap> pixmap;
};

struct particleprops final {
  float x, y;
  float hw, hh;
  bool spawning;
  rng::uniform_real<float> xspawnd, yspawnd, radiusd, angled;
  rng::uniform_real<float> xveld, yveld, gxd, gyd;
  rng::uniform_real<float> scaled, lifed, rotforced, rotveld;

  float randradius() noexcept { return radiusd(rng::engine::global()); }
  float randangle() noexcept { return angled(rng::engine::global()); }
  float randxspawn() noexcept { return xspawnd(rng::engine::global()); }
  float randyspawn() noexcept { return yspawnd(rng::engine::global()); }
  float randxvel() noexcept { return xveld(rng::engine::global()); }
  float randyvel() noexcept { return yveld(rng::engine::global()); }
  float randgx() noexcept { return gxd(rng::engine::global()); }
  float randgy() noexcept { return gyd(rng::engine::global()); }
  float randscale() noexcept { return scaled(rng::engine::global()); }
  float randlife() noexcept { return lifed(rng::engine::global()); }
  float randrotforce() noexcept { return rotforced(rng::engine::global()); }
  float randrotvel() noexcept { return rotveld(rng::engine::global()); }

  void set_position(float xv, float yv) noexcept { x = xv; y = yv; }
};

struct particles final {
  std::vector<float> x, y, vx, vy, gx, gy;
  std::vector<float> life, scale, angle, av, af;
  size_t count{0};

  void reserve(size_t n) {
    x.reserve(n); y.reserve(n);
    vx.reserve(n); vy.reserve(n);
    gx.reserve(n); gy.reserve(n);
    life.reserve(n); scale.reserve(n);
    angle.reserve(n); av.reserve(n); af.reserve(n);
  }

  void resize(size_t n) {
    if (x.capacity() < n) reserve(n);
    count = n;
    x.resize(n); y.resize(n);
    vx.resize(n); vy.resize(n);
    gx.resize(n); gy.resize(n);
    life.resize(n); scale.resize(n);
    angle.resize(n); av.resize(n); af.resize(n);
  }
};

struct particlebatch final {
  std::shared_ptr<particleprops> props;
  std::shared_ptr<pixmap> pixmap;
  std::vector<int> indices;
  std::vector<SDL_Vertex> vertices;
  std::vector<size_t> respawn;
  particles particles;

  [[nodiscard]] size_t size() const noexcept { return particles.count; }
};

class particlepool final {
public:
  particlepool(entt::registry& registry);
  ~particlepool() = default;

  void add(unmarshal::json node, int32_t z);

  void populate(sol::table& pool) const;

  void clear();

  void update(float delta);

  void draw(entt::entity entity) const noexcept;

private:
  entt::registry& _registry;
  mutable boost::unordered_flat_map<std::string, cache, transparent_string_hash, std::equal_to<>> _cache;
  boost::unordered_flat_map<std::string, std::pair<entt::entity, std::shared_ptr<particlebatch>>, transparent_string_hash, std::equal_to<>> _batches;
};
