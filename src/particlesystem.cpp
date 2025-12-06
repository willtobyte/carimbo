#include "particlesystem.hpp"

#include "io.hpp"
#include "pixmap.hpp"
#include "resourcemanager.hpp"

particlefactory::particlefactory(std::shared_ptr<resourcemanager> resourcemanager)
    : _resourcemanager(std::move(resourcemanager)) {
}

std::shared_ptr<particlebatch> particlefactory::create(std::string_view kind, float x, float y, bool spawning) const {
  const auto filename = std::format("particles/{}.json", kind);
  auto document = unmarshal::parse(io::read(filename));

  const auto pixmap = _resourcemanager->pixmappool()->get(std::format("blobs/particles/{}.png", kind));

  const auto count = unmarshal::value_or(document, "count", 0uz);

  const auto spawn = unmarshal::find_object(document, "spawn");
  const auto velocity = unmarshal::find_object(document, "velocity");
  const auto gravity = unmarshal::find_object(document, "gravity");
  const auto rotation = unmarshal::find_object(document, "rotation");

  const auto [xspawn_start, xspawn_end] = unmarshal::range<float>(spawn, "x", .0f, .0f);
  const auto [yspawn_start, yspawn_end] = unmarshal::range<float>(spawn, "y", .0f, .0f);
  const auto [radius_start, radius_end] = unmarshal::range<float>(spawn, "radius", .0f, .0f);
  const auto [angle_start, angle_end] = unmarshal::range<double>(spawn, "angle", .0, .0);
  const auto [scale_start, scale_end] = unmarshal::range<float>(spawn, "scale", 1.0f, 1.0f);
  const auto [life_start, life_end] = unmarshal::range<float>(spawn, "life", 1.0f, 1.0f);
  const auto [alpha_start, alpha_end] = unmarshal::range<unsigned int>(spawn, "alpha", 255u, 255u);
  const auto [xvel_start, xvel_end] = unmarshal::range<float>(velocity, "x", .0f, .0f);
  const auto [yvel_start, yvel_end] = unmarshal::range<float>(velocity, "y", .0f, .0f);
  const auto [gx_start, gx_end] = unmarshal::range<float>(gravity, "x", .0f, .0f);
  const auto [gy_start, gy_end] = unmarshal::range<float>(gravity, "y", .0f, .0f);
  const auto [rforce_start, rforce_end] = unmarshal::range<double>(rotation, "force", .0, .0);
  const auto [rvel_start, rvel_end] = unmarshal::range<double>(rotation, "velocity", .0, .0);

  const auto props = std::make_shared<particleprops>();
  props->active = true;
  props->spawning = spawning;
  props->x = x;
  props->y = y;
  props->pixmap = pixmap;
  props->xspawnd = std::uniform_real_distribution<float>(xspawn_start, xspawn_end);
  props->yspawnd = std::uniform_real_distribution<float>(yspawn_start, yspawn_end);
  props->radiusd = std::uniform_real_distribution<float>(radius_start, radius_end);
  props->angled = std::uniform_real_distribution<double>(angle_start, angle_end);
  props->xveld = std::uniform_real_distribution<float>(xvel_start, xvel_end);
  props->yveld = std::uniform_real_distribution<float>(yvel_start, yvel_end);
  props->gxd = std::uniform_real_distribution<float>(gx_start, gx_end);
  props->gyd = std::uniform_real_distribution<float>(gy_start, gy_end);
  props->lifed = std::uniform_real_distribution<float>(life_start, life_end);
  props->alphad = std::uniform_int_distribution<unsigned int>(alpha_start, alpha_end);
  props->scaled = std::uniform_real_distribution<float>(scale_start, scale_end);
  props->rotforced = std::uniform_real_distribution<double>(rforce_start, rforce_end);
  props->rotveld = std::uniform_real_distribution<double>(rvel_start, rvel_end);

  auto batch = std::make_shared<particlebatch>();
  batch->props = std::move(props);
  batch->particles.resize(count);
  batch->vertices.resize(count * 4);

  batch->indices.resize(count * 6);
  for (auto i = 0uz; i < count; ++i) {
    const auto base = static_cast<int>(i * 4);
    const auto index = i * 6;
    batch->indices[index] = base;
    batch->indices[index + 1] = base + 1;
    batch->indices[index + 2] = base + 2;
    batch->indices[index + 3] = base;
    batch->indices[index + 4] = base + 2;
    batch->indices[index + 5] = base + 3;
  }

  return batch;
}

particlesystem::particlesystem(std::shared_ptr<resourcemanager> resourcemanager)
    : _renderer(resourcemanager->renderer()),
      _factory(std::make_shared<particlefactory>(resourcemanager)) {
  _batches.reserve(16);
}

void particlesystem::add(const std::shared_ptr<particlebatch>& batch) {
  if (!batch) [[unlikely]] {
    return;
  }

  _batches.emplace_back(batch);
}

void particlesystem::set(const std::vector<std::shared_ptr<particlebatch>>& batches) {
  if (batches.empty()) {
    _batches.clear();
    return;
  }

  _batches = batches;
}

void particlesystem::clear() {
  _batches.clear();
}

void particlesystem::update(float delta) {
  const auto d = static_cast<double>(delta);

  for (const auto& batch : _batches) {
    auto& props = batch->props;
    if (!props->active) [[unlikely]] {
      continue;
    }

    auto* particles = batch->particles.data();
    const auto n = batch->size();

    for (auto i = 0uz; i < n; ++i) {
      auto& p = particles[i];

      p.life -= delta;
      if (p.life > 0.f) {
        p.av += p.af * d;
        p.angle += p.av * d;

        p.vx += p.gx * delta;
        p.vy += p.gy * delta;
        p.x += p.vx * delta;
        p.y += p.vy * delta;

        const auto a = 255.f * p.life;
        p.alpha = static_cast<uint8_t>(std::clamp(a, .0f, 255.f));
        continue;
      }

      if (!props->spawning) [[unlikely]] {
        continue;
      }

      p.x = props->x + props->randxspawn();
      p.y = props->y + props->randyspawn();
      p.vx = props->randxvel();
      p.vy = props->randyvel();
      p.gx = props->randgx();
      p.gy = props->randgy();
      p.av = props->randrotvel();
      p.af = props->randrotforce();
      p.life = props->randlife();
      p.alpha = props->randalpha();
      p.scale = props->randscale();
      p.angle = props->randangle();
    }
  }
}

void particlesystem::draw() const {
  static constexpr auto u = std::array{.0f, 1.f, 1.f, .0f};
  static constexpr auto v = std::array{.0f, .0f, 1.f, 1.f};

  for (const auto& batch : _batches) {
    const auto& props = batch->props;
    if (!props->active) [[unlikely]] {
      continue;
    }

    const auto& pixmap = *props->pixmap;
    const auto hw = static_cast<float>(pixmap.width()) * 0.5f;
    const auto hh = static_cast<float>(pixmap.height()) * 0.5f;
    const auto n = batch->size();

    const auto* particles = batch->particles.data();
    auto* vertices = batch->vertices.data();

    for (auto i = 0uz; i < n; ++i) {
      const auto& p = particles[i];

      const auto shw = hw * p.scale;
      const auto shh = hh * p.scale;

      const auto ca = static_cast<float>(std::cos(p.angle));
      const auto sa = static_cast<float>(std::sin(p.angle));

      const auto lx = std::array{-shw, shw, shw, -shw};
      const auto ly = std::array{-shh, -shh, shh, shh};

      const SDL_FColor color = {1.f, 1.f, 1.f, p.alpha / 255.f};

      auto* vx = vertices + i * 4;

      for (auto j = 0uz; j < 4; ++j) {
        const auto rx = lx[j] * ca - ly[j] * sa;
        const auto ry = lx[j] * sa + ly[j] * ca;

        vx[j].position.x = p.x + rx;
        vx[j].position.y = p.y + ry;
        vx[j].tex_coord.x = u[j];
        vx[j].tex_coord.y = v[j];
        vx[j].color = color;
      }
    }

    SDL_RenderGeometry(
        *_renderer,
        static_cast<SDL_Texture*>(pixmap),
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
