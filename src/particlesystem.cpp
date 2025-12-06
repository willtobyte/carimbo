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

  auto spawn = unmarshal::find_object(document, "spawn");
  auto velocity = unmarshal::find_object(document, "velocity");
  auto gravity = unmarshal::find_object(document, "gravity");
  auto rotation = unmarshal::find_object(document, "rotation");

  auto radius = spawn ? unmarshal::find_object(*spawn, "radius") : std::nullopt;
  auto angle = spawn ? unmarshal::find_object(*spawn, "angle") : std::nullopt;
  auto xspawn = spawn ? unmarshal::find_object(*spawn, "x") : std::nullopt;
  auto yspawn = spawn ? unmarshal::find_object(*spawn, "y") : std::nullopt;
  auto scale = spawn ? unmarshal::find_object(*spawn, "scale") : std::nullopt;
  auto life = spawn ? unmarshal::find_object(*spawn, "life") : std::nullopt;
  auto alpha = spawn ? unmarshal::find_object(*spawn, "alpha") : std::nullopt;
  auto xvel = velocity ? unmarshal::find_object(*velocity, "x") : std::nullopt;
  auto yvel = velocity ? unmarshal::find_object(*velocity, "y") : std::nullopt;
  auto gx = gravity ? unmarshal::find_object(*gravity, "x") : std::nullopt;
  auto gy = gravity ? unmarshal::find_object(*gravity, "y") : std::nullopt;
  auto rforce = rotation ? unmarshal::find_object(*rotation, "force") : std::nullopt;
  auto rvel = rotation ? unmarshal::find_object(*rotation, "velocity") : std::nullopt;

  const auto props = std::make_shared<particleprops>();
  props->active = true;
  props->spawning = spawning;
  props->x = x;
  props->y = y;
  props->pixmap = pixmap;
  props->xspawnd = std::uniform_real_distribution<float>(xspawn ? unmarshal::value_or(*xspawn, "start", .0f) : .0f, xspawn ? unmarshal::value_or(*xspawn, "end", .0f) : .0f);
  props->yspawnd = std::uniform_real_distribution<float>(yspawn ? unmarshal::value_or(*yspawn, "start", .0f) : .0f, yspawn ? unmarshal::value_or(*yspawn, "end", .0f) : .0f);
  props->radiusd = std::uniform_real_distribution<float>(radius ? unmarshal::value_or(*radius, "start", .0f) : .0f, radius ? unmarshal::value_or(*radius, "end", .0f) : .0f);
  props->angled = std::uniform_real_distribution<double>(angle ? unmarshal::value_or(*angle, "start", .0) : .0, angle ? unmarshal::value_or(*angle, "end", .0) : .0);
  props->xveld = std::uniform_real_distribution<float>(xvel ? unmarshal::value_or(*xvel, "start", .0f) : .0f, xvel ? unmarshal::value_or(*xvel, "end", .0f) : .0f);
  props->yveld = std::uniform_real_distribution<float>(yvel ? unmarshal::value_or(*yvel, "start", .0f) : .0f, yvel ? unmarshal::value_or(*yvel, "end", .0f) : .0f);
  props->gxd = std::uniform_real_distribution<float>(gx ? unmarshal::value_or(*gx, "start", .0f) : .0f, gx ? unmarshal::value_or(*gx, "end", .0f) : .0f);
  props->gyd = std::uniform_real_distribution<float>(gy ? unmarshal::value_or(*gy, "start", .0f) : .0f, gy ? unmarshal::value_or(*gy, "end", .0f) : .0f);
  props->lifed = std::uniform_real_distribution<float>(life ? unmarshal::value_or(*life, "start", 1.0f) : 1.0f, life ? unmarshal::value_or(*life, "end", 1.0f) : 1.0f);
  props->alphad = std::uniform_int_distribution<unsigned int>(alpha ? unmarshal::value_or(*alpha, "start", 255u) : 255u, alpha ? unmarshal::value_or(*alpha, "end", 255u) : 255u);
  props->scaled = std::uniform_real_distribution<float>(scale ? unmarshal::value_or(*scale, "start", 1.0f) : 1.0f, scale ? unmarshal::value_or(*scale, "end", 1.0f) : 1.0f);
  props->rotforced = std::uniform_real_distribution<double>(rforce ? unmarshal::value_or(*rforce, "start", .0) : .0, rforce ? unmarshal::value_or(*rforce, "end", .0) : .0);
  props->rotveld = std::uniform_real_distribution<double>(rvel ? unmarshal::value_or(*rvel, "start", .0) : .0, rvel ? unmarshal::value_or(*rvel, "end", .0) : .0);

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
