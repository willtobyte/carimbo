#include "particlesystem.hpp"

#include "io.hpp"
#include "pixmap.hpp"
#include "resourcemanager.hpp"

particlefactory::particlefactory(std::shared_ptr<resourcemanager> resourcemanager)
    : _resourcemanager(std::move(resourcemanager)) {
}

std::shared_ptr<particlebatch> particlefactory::create(std::string_view kind, float x, float y, bool spawning) const {
  const auto filename = std::format("particles/{}.json", kind);
  const auto buffer = io::read(filename);
  const auto j = nlohmann::json::parse(buffer);

  const auto pixmap = _resourcemanager->pixmappool()->get(std::format("blobs/particles/{}.png", kind));

  const auto count = j.value("count", 0uz);

  const auto spawn = j.value("spawn", nlohmann::json::object());
  const auto velocity = j.value("velocity", nlohmann::json::object());
  const auto gravity = j.value("gravity", nlohmann::json::object());
  const auto rotation = j.value("rotation", nlohmann::json::object());

  const auto radius = spawn.value("radius", nlohmann::json::object());
  const auto angle = spawn.value("angle", nlohmann::json::object());
  const auto xspawn = spawn.value("x", nlohmann::json::object());
  const auto yspawn = spawn.value("y", nlohmann::json::object());
  const auto scale = spawn.value("scale", nlohmann::json::object());
  const auto life = spawn.value("life", nlohmann::json::object());
  const auto alpha = spawn.value("alpha", nlohmann::json::object());
  const auto xvel = velocity.value("x", nlohmann::json::object());
  const auto yvel = velocity.value("y", nlohmann::json::object());
  const auto gx = gravity.value("x", nlohmann::json::object());
  const auto gy = gravity.value("y", nlohmann::json::object());
  const auto rforce = rotation.value("force", nlohmann::json::object());
  const auto rvel = rotation.value("velocity", nlohmann::json::object());

  const auto ps = std::make_shared<particleprops>();
  ps->active = true;
  ps->spawning = spawning;
  ps->x = x;
  ps->y = y;
  ps->pixmap = pixmap;
  ps->xspawnd = std::uniform_real_distribution<float>(xspawn.value("start", .0f), xspawn.value("end", .0f));
  ps->yspawnd = std::uniform_real_distribution<float>(yspawn.value("start", .0f), yspawn.value("end", .0f));
  ps->radiusd = std::uniform_real_distribution<float>(radius.value("start", .0f), radius.value("end", .0f));
  ps->angled = std::uniform_real_distribution<double>(angle.value("start", .0), angle.value("end", .0));
  ps->xveld = std::uniform_real_distribution<float>(xvel.value("start", .0f), xvel.value("end", .0f));
  ps->yveld = std::uniform_real_distribution<float>(yvel.value("start", .0f), yvel.value("end", .0f));
  ps->gxd = std::uniform_real_distribution<float>(gx.value("start", .0f), gx.value("end", .0f));
  ps->gyd = std::uniform_real_distribution<float>(gy.value("start", .0f), gy.value("end", .0f));
  ps->lifed = std::uniform_real_distribution<float>(life.value("start", 1.0f), life.value("end", 1.0f));
  ps->alphad = std::uniform_int_distribution<unsigned int>(alpha.value("start", 255u), alpha.value("end", 255u));
  ps->scaled = std::uniform_real_distribution<float>(scale.value("start", 1.0f), scale.value("end", 1.0f));
  ps->rotforced = std::uniform_real_distribution<double>(rforce.value("start", .0), rforce.value("end", .0));
  ps->rotveld = std::uniform_real_distribution<double>(rvel.value("start", .0), rvel.value("end", .0));

  auto pb = std::make_shared<particlebatch>();
  pb->props = std::move(ps);
  pb->particles.resize(count);

  return pb;
}

particlesystem::particlesystem(std::shared_ptr<resourcemanager> resourcemanager)
    : _renderer(resourcemanager->renderer()),
      _factory(std::make_shared<particlefactory>(resourcemanager)) {
  _batches.reserve(16);
  _vertices.reserve(8096);
  _indices.reserve(10240);
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

    for (auto i = n; i-- > 0uz;) {
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
  for (const auto& batch : _batches) {
    const auto& props = batch->props;
    if (!props->active) [[unlikely]] {
      continue;
    }

    const auto& pixmap = *props->pixmap;
    const auto tw = static_cast<float>(pixmap.width());
    const auto th = static_cast<float>(pixmap.height());
    const auto hw = tw * 0.5f;
    const auto hh = th * 0.5f;
    const auto n = batch->size();

    _vertices.clear();
    _indices.clear();

    const auto required_vertices = n * 4;
    const auto required_indices = n * 6;

    if (_vertices.capacity() < required_vertices) {
      _vertices.reserve(required_vertices);
    }

    if (_indices.capacity() < required_indices) {
      _indices.reserve(required_indices);
    }

    const auto* particles = batch->particles.data();
    auto vi = 0;

    for (auto i = 0uz; i < n; ++i) {
      const auto& p = particles[i];

      if (p.life <= 0.f) [[unlikely]] {
        continue;
      }

      const auto shw = hw * p.scale;
      const auto shh = hh * p.scale;

      const auto cos_a = static_cast<float>(std::cos(p.angle));
      const auto sin_a = static_cast<float>(std::sin(p.angle));

      const float lx[4] = {-shw, shw, shw, -shw};
      const float ly[4] = {-shh, -shh, shh, shh};

      const float u[4] = {0.f, 1.f, 1.f, 0.f};
      const float v[4] = {0.f, 0.f, 1.f, 1.f};

      const SDL_FColor color = {1.f, 1.f, 1.f, p.alpha / 255.f};

      const auto base = vi;

      for (int j = 0; j < 4; ++j) {
        const float rx = lx[j] * cos_a - ly[j] * sin_a;
        const float ry = lx[j] * sin_a + ly[j] * cos_a;

        SDL_Vertex vertex;
        vertex.position.x = p.x + rx;
        vertex.position.y = p.y + ry;
        vertex.tex_coord.x = u[j];
        vertex.tex_coord.y = v[j];
        vertex.color = color;

        _vertices.emplace_back(vertex);
      }

      _indices.emplace_back(base);
      _indices.emplace_back(base + 1);
      _indices.emplace_back(base + 2);
      _indices.emplace_back(base);
      _indices.emplace_back(base + 2);
      _indices.emplace_back(base + 3);

      vi += 4;
    }

    SDL_RenderGeometry(
        *_renderer,
        static_cast<SDL_Texture*>(pixmap),
        _vertices.data(),
        static_cast<int>(_vertices.size()),
        _indices.data(),
        static_cast<int>(_indices.size())
    );
  }
}

std::shared_ptr<particlefactory> particlesystem::factory() const noexcept {
  return _factory;
}
