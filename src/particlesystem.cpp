#include "particlesystem.hpp"

#include "io.hpp"
#include "pixmap.hpp"
#include "renderer.hpp"

namespace {

constexpr float TWO_PI = 6.28318530718f;
constexpr float HALF_PI = 1.57079632679f;
constexpr float INV_HALF_PI = 0.63661977236f;

constexpr float SIN_C0 = 0.99997f;
constexpr float SIN_C1 = 0.16596f;
constexpr float SIN_C2 = 0.00759f;
constexpr float COS_C0 = 0.99996f;
constexpr float COS_C1 = 0.49985f;
constexpr float COS_C2 = 0.03659f;

constexpr float QUADRANT_SIGNS[8] = {1.f, 1.f, 1.f, -1.f, -1.f, -1.f, -1.f, 1.f};
constexpr int QUADRANT_MASK = 3;

static void sincos(float x, float& out_sin, float& out_cos) noexcept {
  const auto q = static_cast<int>(x * INV_HALF_PI);
  const auto t = x - static_cast<float>(q) * HALF_PI;
  const auto t2 = t * t;

  const auto sin_t = t * (SIN_C0 - t2 * (SIN_C1 - t2 * SIN_C2));
  const auto cos_t = COS_C0 - t2 * (COS_C1 - t2 * COS_C2);

  const auto qi = (q & QUADRANT_MASK) * 2;
  const auto swap = static_cast<float>(q & 1);
  const auto keep = 1.f - swap;

  out_sin = (sin_t * keep + cos_t * swap) * QUADRANT_SIGNS[qi];
  out_cos = (cos_t * keep + sin_t * swap) * QUADRANT_SIGNS[qi + 1];
}

template <typename T>
static void range(unmarshal::json node, std::pair<T, T>& out) noexcept {
  if (!node) {
    return;
  }

  out = {node["start"].get(out.first), node["end"].get(out.second)};
}
}

particlefactory::particlefactory(std::shared_ptr<renderer> renderer)
    : _renderer(std::move(renderer)) {}

std::shared_ptr<particlebatch> particlefactory::create(std::string_view kind, float x, float y, bool spawning) const {
  auto [it, inserted] = _cache.try_emplace(kind);
  if (inserted) {
    auto json = unmarshal::parse(io::read(std::format("particles/{}.json", kind)));

    it->second.count = json["count"].get<size_t>();
    it->second.xspawn = {.0f, .0f};
    it->second.yspawn = {.0f, .0f};
    it->second.radius = {.0f, .0f};
    it->second.angle = {.0f, .0f};
    it->second.scale = {1.0f, 1.0f};
    it->second.life = {1.0f, 1.0f};
    it->second.alpha = {uint8_t{255}, uint8_t{255}};
    it->second.xvel = {.0f, .0f};
    it->second.yvel = {.0f, .0f};
    it->second.gx = {.0f, .0f};
    it->second.gy = {.0f, .0f};
    it->second.rforce = {.0f, .0f};
    it->second.rvel = {.0f, .0f};

    if (auto spawn = json["spawn"]) {
      range(spawn["x"], it->second.xspawn);
      range(spawn["y"], it->second.yspawn);
      range(spawn["radius"], it->second.radius);
      range(spawn["angle"], it->second.angle);
      range(spawn["scale"], it->second.scale);
      range(spawn["life"], it->second.life);
      range(spawn["alpha"], it->second.alpha);
    }

    if (auto velocity = json["velocity"]) {
      range(velocity["x"], it->second.xvel);
      range(velocity["y"], it->second.yvel);
    }

    if (auto gravity = json["gravity"]) {
      range(gravity["x"], it->second.gx);
      range(gravity["y"], it->second.gy);
    }

    if (auto rotation = json["rotation"]) {
      range(rotation["force"], it->second.rforce);
      range(rotation["velocity"], it->second.rvel);
    }

    it->second.pixmap = std::make_shared<::pixmap>(_renderer, std::format("blobs/particles/{}.png", kind));
  }

  const auto props = std::make_shared<particleprops>();
  props->spawning = spawning;
  props->x = x;
  props->y = y;
  props->hw = static_cast<float>(it->second.pixmap->width()) * 0.5f;
  props->hh = static_cast<float>(it->second.pixmap->height()) * 0.5f;
  props->xspawnd = rng::uniform_real<float>(it->second.xspawn.first, it->second.xspawn.second);
  props->yspawnd = rng::uniform_real<float>(it->second.yspawn.first, it->second.yspawn.second);
  props->radiusd = rng::uniform_real<float>(it->second.radius.first, it->second.radius.second);
  props->angled = rng::uniform_real<float>(it->second.angle.first, it->second.angle.second);
  props->xveld = rng::uniform_real<float>(it->second.xvel.first, it->second.xvel.second);
  props->yveld = rng::uniform_real<float>(it->second.yvel.first, it->second.yvel.second);
  props->gxd = rng::uniform_real<float>(it->second.gx.first, it->second.gx.second);
  props->gyd = rng::uniform_real<float>(it->second.gy.first, it->second.gy.second);
  props->lifed = rng::uniform_real<float>(it->second.life.first, it->second.life.second);
  props->alphad = rng::uniform_int<unsigned int>(it->second.alpha.first, it->second.alpha.second);
  props->scaled = rng::uniform_real<float>(it->second.scale.first, it->second.scale.second);
  props->rotforced = rng::uniform_real<float>(it->second.rforce.first, it->second.rforce.second);
  props->rotveld = rng::uniform_real<float>(it->second.rvel.first, it->second.rvel.second);

  const auto count = it->second.count;
  const auto batch = std::make_shared<particlebatch>();
  batch->props = std::move(props);
  batch->pixmap = it->second.pixmap;
  batch->particles.resize(count);
  batch->vertices.resize(count * 4);
  batch->indices.resize(count * 6);
  batch->respawn.resize(count);

  for (auto i = 0uz; i < count; ++i) {
    const auto base = static_cast<int>(i * 4);
    const auto index = i * 6uz;
    batch->indices[index] = base;
    batch->indices[index + 1] = base + 1;
    batch->indices[index + 2] = base + 2;
    batch->indices[index + 3] = base;
    batch->indices[index + 4] = base + 2;
    batch->indices[index + 5] = base + 3;
  }

  return batch;
}

particlesystem::particlesystem(std::shared_ptr<renderer> renderer)
    : _renderer(std::move(renderer)),
      _factory(std::make_shared<particlefactory>(_renderer)) {
  _batches.reserve(16);
}

void particlesystem::add(unmarshal::json node) {
  const auto name = node["name"].get<std::string_view>();
  auto [it, inserted] = _batches.try_emplace(name);
  if (!inserted) {
    return;
  }

  const auto kind = node["kind"].get<std::string_view>();
  const auto x = node["x"].get<float>();
  const auto y = node["y"].get<float>();
  const auto spawning = node["spawning"].get(true);
  it->second = _factory->create(kind, x, y, spawning);
}

void particlesystem::populate(sol::table& pool) const {
  for (const auto& [name, batch] : _batches) {
    assert(!pool[name].valid() && "duplicate key in pool");
    pool[name] = batch->props;
  }
}

void particlesystem::clear() {
  _batches.clear();
}

void particlesystem::update(float delta) {
  for (const auto& [_, batch] : _batches) {
    auto* props = batch->props.get();
    auto& p = batch->particles;
    const auto n = p.count;

    auto* __restrict xs = p.x.data();
    auto* __restrict ys = p.y.data();
    auto* __restrict vxs = p.vx.data();
    auto* __restrict vys = p.vy.data();
    auto* __restrict gxs = p.gx.data();
    auto* __restrict gys = p.gy.data();
    auto* __restrict lifes = p.life.data();
    auto* __restrict scales = p.scale.data();
    auto* __restrict angles = p.angle.data();
    auto* __restrict avs = p.av.data();
    auto* __restrict afs = p.af.data();
    auto* __restrict alphas = p.alpha.data();

    for (size_t i = 0; i < n; ++i) {
      lifes[i] -= delta;
      avs[i] += afs[i] * delta;
      angles[i] += avs[i] * delta;
      angles[i] -= TWO_PI * static_cast<float>(angles[i] >= TWO_PI);
      angles[i] += TWO_PI * static_cast<float>(angles[i] < .0f);
      vxs[i] += gxs[i] * delta;
      vys[i] += gys[i] * delta;
      xs[i] += vxs[i] * delta;
      ys[i] += vys[i] * delta;
    }

    if (props->spawning) {
      const auto px = props->x;
      const auto py = props->y;

      auto* __restrict respawn = batch->respawn.data();
      size_t count = 0;

      for (size_t i = 0; i < n; ++i) {
        respawn[count] = i;
        count += static_cast<size_t>(lifes[i] <= 0.f);
      }

      for (size_t j = 0; j < count; ++j) {
        const auto i = respawn[j];
        xs[i] = px + props->randxspawn();
        ys[i] = py + props->randyspawn();
        vxs[i] = props->randxvel();
        vys[i] = props->randyvel();
        gxs[i] = props->randgx();
        gys[i] = props->randgy();
        avs[i] = props->randrotvel();
        afs[i] = props->randrotforce();
        lifes[i] = props->randlife();
        alphas[i] = props->randalpha();
        scales[i] = props->randscale();
        angles[i] = props->randangle();
      }
    }

    const auto hw = props->hw;
    const auto hh = props->hh;
    auto* vertices = batch->vertices.data();

    for (auto i = 0uz; i < n; ++i) {
      const auto life = lifes[i];
      const auto alive = life > 0.f ? 1.f : 0.f;
      const auto alpha = std::min(life, 1.f) * alive;

      const auto scale = scales[i];
      const auto shw = hw * scale;
      const auto shh = hh * scale;

      float sa, ca;
      sincos(angles[i], sa, ca);

      const auto x = xs[i];
      const auto y = ys[i];
      const SDL_FColor color = {1.f, 1.f, 1.f, alpha};

      const auto dx0 = -shw * ca + shh * sa;
      const auto dy0 = -shw * sa - shh * ca;
      const auto dx1 = shw * ca + shh * sa;
      const auto dy1 = shw * sa - shh * ca;

      auto* vx = vertices + i * 4;
      vx[0] = {{x + dx0, y + dy0}, color, {0.f, 0.f}};
      vx[1] = {{x + dx1, y + dy1}, color, {1.f, 0.f}};
      vx[2] = {{x - dx0, y - dy0}, color, {1.f, 1.f}};
      vx[3] = {{x - dx1, y - dy1}, color, {0.f, 1.f}};
    }
  }
}

void particlesystem::draw() const {
  for (const auto& [_, batch] : _batches) {
    SDL_RenderGeometry(
        *_renderer,
        static_cast<SDL_Texture*>(*batch->pixmap),
        batch->vertices.data(),
        static_cast<int>(batch->vertices.size()),
        batch->indices.data(),
        static_cast<int>(batch->indices.size())
    );
  }
}

std::shared_ptr<particlefactory> particlesystem::factory() const noexcept {
  return _factory;
}
