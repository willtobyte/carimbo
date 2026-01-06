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
std::pair<T, T> read_range(unmarshal::value node, T fallback_start, T fallback_end) noexcept {
  return {
    unmarshal::get_or(node, "start", fallback_start),
    unmarshal::get_or(node, "end", fallback_end)
  };
}

template <typename T>
std::pair<T, T> read_range_from(unmarshal::value parent, const char* key, T fallback_start, T fallback_end) noexcept {
  if (auto node = unmarshal::child(parent, key)) {
    return read_range(node, fallback_start, fallback_end);
  }

  return {fallback_start, fallback_end};
}

struct particleconfig final {
  size_t count{0};

  std::pair<float, float> xspawn{.0f, .0f};
  std::pair<float, float> yspawn{.0f, .0f};
  std::pair<float, float> radius{.0f, .0f};
  std::pair<float, float> angle{.0f, .0f};
  std::pair<float, float> scale{1.0f, 1.0f};
  std::pair<float, float> life{1.0f, 1.0f};
  std::pair<uint8_t, uint8_t> alpha{255u, 255u};

  std::pair<float, float> xvel{.0f, .0f};
  std::pair<float, float> yvel{.0f, .0f};

  std::pair<float, float> gx{.0f, .0f};
  std::pair<float, float> gy{.0f, .0f};

  std::pair<float, float> rforce{.0f, .0f};
  std::pair<float, float> rvel{.0f, .0f};

  friend void from_json(unmarshal::value node, particleconfig& out) {
    out.count = static_cast<size_t>(unmarshal::get_or(node, "count", uint64_t{0}));

    if (auto spawn = unmarshal::child(node, "spawn")) {
      out.xspawn = read_range_from(spawn, "x", .0f, .0f);
      out.yspawn = read_range_from(spawn, "y", .0f, .0f);
      out.radius = read_range_from(spawn, "radius", .0f, .0f);
      out.angle = read_range_from(spawn, "angle", .0f, .0f);
      out.scale = read_range_from(spawn, "scale", 1.0f, 1.0f);
      out.life = read_range_from(spawn, "life", 1.0f, 1.0f);
      out.alpha = read_range_from(spawn, "alpha", uint8_t{255}, uint8_t{255});
    }

    if (auto velocity = unmarshal::child(node, "velocity")) {
      out.xvel = read_range_from(velocity, "x", .0f, .0f);
      out.yvel = read_range_from(velocity, "y", .0f, .0f);
    }

    if (auto gravity = unmarshal::child(node, "gravity")) {
      out.gx = read_range_from(gravity, "x", .0f, .0f);
      out.gy = read_range_from(gravity, "y", .0f, .0f);
    }

    if (auto rotation = unmarshal::child(node, "rotation")) {
      out.rforce = read_range_from(rotation, "force", .0f, .0f);
      out.rvel = read_range_from(rotation, "velocity", .0f, .0f);
    }
  }
};
}

particlefactory::particlefactory(std::shared_ptr<renderer> renderer)
    : _renderer(std::move(renderer)) {}

std::shared_ptr<particlebatch> particlefactory::create(std::string_view kind, float x, float y, bool spawning) const {
  auto json = unmarshal::parse(io::read(std::format("particles/{}.json", kind)));
  particleconfig conf;
  from_json(*json, conf);

  auto [iterator, inserted] = _pixmaps.try_emplace(std::string{kind});
  if (inserted) {
    iterator->second = std::make_shared<::pixmap>(_renderer, std::format("blobs/particles/{}.png", kind));
  }

  const auto& pixmap = iterator->second;

  const auto props = std::make_shared<particleprops>();
  props->spawning = spawning;
  props->x = x;
  props->y = y;
  props->hw = static_cast<float>(pixmap->width()) * 0.5f;
  props->hh = static_cast<float>(pixmap->height()) * 0.5f;
  props->pixmap = pixmap;
  props->xspawnd = rng::uniform_real<float>(conf.xspawn.first, conf.xspawn.second);
  props->yspawnd = rng::uniform_real<float>(conf.yspawn.first, conf.yspawn.second);
  props->radiusd = rng::uniform_real<float>(conf.radius.first, conf.radius.second);
  props->angled = rng::uniform_real<float>(conf.angle.first, conf.angle.second);
  props->xveld = rng::uniform_real<float>(conf.xvel.first, conf.xvel.second);
  props->yveld = rng::uniform_real<float>(conf.yvel.first, conf.yvel.second);
  props->gxd = rng::uniform_real<float>(conf.gx.first, conf.gx.second);
  props->gyd = rng::uniform_real<float>(conf.gy.first, conf.gy.second);
  props->lifed = rng::uniform_real<float>(conf.life.first, conf.life.second);
  props->alphad = rng::uniform_int<unsigned int>(conf.alpha.first, conf.alpha.second);
  props->scaled = rng::uniform_real<float>(conf.scale.first, conf.scale.second);
  props->rotforced = rng::uniform_real<float>(conf.rforce.first, conf.rforce.second);
  props->rotveld = rng::uniform_real<float>(conf.rvel.first, conf.rvel.second);

  const auto batch = std::make_shared<particlebatch>();
  batch->props = std::move(props);
  batch->particles.resize(conf.count);
  batch->vertices.resize(conf.count * 4);
  batch->indices.resize(conf.count * 6);
  batch->respawn.resize(conf.count);

  for (auto i = 0uz; i < conf.count; ++i) {
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

void particlesystem::add(unmarshal::value particle) {
  const auto name = unmarshal::get<std::string_view>(particle, "name");
  const auto kind = unmarshal::get<std::string_view>(particle, "kind");
  const auto x = unmarshal::get<float>(particle, "x");
  const auto y = unmarshal::get<float>(particle, "y");
  const auto spawning = unmarshal::get_or(particle, "spawning", true);
  _batches.emplace(name, _factory->create(kind, x, y, spawning));
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
    const auto& props = batch->props;

    SDL_RenderGeometry(
        *_renderer,
        static_cast<SDL_Texture*>(*props->pixmap),
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
