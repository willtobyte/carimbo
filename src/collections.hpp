#pragma once

#include "common.hpp"

#include "random.hpp"

struct particleprops final {
  float x, y;
  float hw, hh;
  bool spawning;
  std::shared_ptr<pixmap> pixmap;
  rng::xorshift128plus rng{rng::engine()()};
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

class effects final {
public:
  effects(std::shared_ptr<soundmanager> soundmanager, std::string_view scenename);
  ~effects() noexcept;

  void add(std::string_view name);

  void populate(sol::table& pool) const;

  void stop() const noexcept;

  void clear();

private:
  std::shared_ptr<soundmanager> _soundmanager;
  std::string _scenename;
  boost::unordered_flat_map<std::string, std::shared_ptr<soundfx>, transparent_string_hash, std::equal_to<>> _effects;
};

class particles final {
public:
  explicit particles(std::shared_ptr<resourcemanager> resourcemanager);
  ~particles() = default;

  void add(unmarshal::object& particle);

  void populate(sol::table& pool) const;

  void clear();

  void update(float delta);

  void draw() const;

  std::shared_ptr<particlefactory> factory() const noexcept;

private:
  std::shared_ptr<renderer> _renderer;
  std::shared_ptr<particlefactory> _factory;
  boost::unordered_flat_map<std::string, std::shared_ptr<particlebatch>, transparent_string_hash, std::equal_to<>> _batches;
};

class objects final {
public:
  objects(
      entt::registry& registry,
      std::shared_ptr<pixmappool> pixmappool,
      std::string_view scenename,
      sol::environment& environment
  );
  ~objects() noexcept = default;

  void add(unmarshal::object& object, int32_t z);

  void populate(sol::table& pool) const;

  void sort();

private:
  entt::registry& _registry;
  std::shared_ptr<pixmappool> _pixmappool;
  std::string _scenename;
  sol::environment& _environment;
  boost::unordered_flat_map<std::string, std::shared_ptr<entityproxy>, transparent_string_hash, std::equal_to<>> _proxies;
};
