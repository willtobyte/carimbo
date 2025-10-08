#include "particlesystem.hpp"

using namespace graphics;

particlefactory::particlefactory(std::shared_ptr<framework::resourcemanager> resourcemanager) noexcept
  : _resourcemanager(std::move(resourcemanager)) {
}

std::shared_ptr<particlebatch> particlefactory::create(const std::string& kind, float_t x, float_t y) const {
  const auto& filename = std::format("particles/{}.json", kind);
  const auto& buffer = storage::io::read(filename);
  const auto& j = nlohmann::json::parse(buffer);

  const auto pixmap = _resourcemanager->pixmappool()->get(std::format("blobs/particles/{}.png", kind));

  const auto count = j.value("count", 0uz);

  const auto& spawn = j.value("spawn", nlohmann::json::object());
  const auto& velocity = j.value("velocity", nlohmann::json::object());
  const auto& gravity = j.value("gravity", nlohmann::json::object());
  const auto& rotation = j.value("rotation", nlohmann::json::object());

  const auto& radius = spawn.value("radius", nlohmann::json::object());
  const auto& angle = spawn.value("angle", nlohmann::json::object());
  const auto& xspawn = spawn.value("x", nlohmann::json::object());
  const auto& yspawn = spawn.value("y", nlohmann::json::object());
  const auto& xvel = velocity.value("x", nlohmann::json::object());
  const auto& yvel = velocity.value("y", nlohmann::json::object());
  const auto& gx = gravity.value("x", nlohmann::json::object());
  const auto& gy = gravity.value("y", nlohmann::json::object());
  const auto& scale = j.value("scale", nlohmann::json::object());
  const auto& life = j.value("life", nlohmann::json::object());
  const auto& alpha = j.value("alpha", nlohmann::json::object());
  const auto& rforce = rotation.value("force", nlohmann::json::object());
  const auto& rvel = rotation.value("velocity", nlohmann::json::object());

  const auto c = std::make_shared<particleconf>();
  c->active = true;
  c->x = x;
  c->y = y;
  c->pixmap = pixmap;
  c->xstartdist = std::uniform_real_distribution<float>(xspawn.value("start", .0f), xspawn.value("end", .0f));
  c->ystartdist = std::uniform_real_distribution<float>(yspawn.value("start", .0f), yspawn.value("end", .0f));
  c->radiusdist = std::uniform_real_distribution<float>(radius.value("start", .0f), radius.value("end", .0f));
  c->angledist = std::uniform_real_distribution<double>(angle.value("start", .0), angle.value("end", .0));
  c->xveldist = std::uniform_real_distribution<float>(xvel.value("start", .0f), xvel.value("end", .0f));
  c->yveldist = std::uniform_real_distribution<float>(yvel.value("start", .0f), yvel.value("end", .0f));
  c->gxdist = std::uniform_real_distribution<float>(gx.value("start", .0f), gx.value("end", .0f));
  c->gydist = std::uniform_real_distribution<float>(gy.value("start", .0f), gy.value("end", .0f));
  c->lifedist = std::uniform_real_distribution<float>(life.value("start", 1.0f), life.value("end", 1.0f));
  c->alphadist = std::uniform_int_distribution<unsigned int>(alpha.value("start", 255u), alpha.value("end", 255u));
  c->scaledist = std::uniform_real_distribution<float>(scale.value("start", 1.0f), scale.value("end", 1.0f));
  c->rotforcedist = std::uniform_real_distribution<float>(rforce.value("start", .0f), rforce.value("end", .0f));
  c->rotveldist = std::uniform_real_distribution<float>(rvel.value("start", .0f),   rvel.value("end", .0f));

  auto ps = std::vector<particle>();
  ps.reserve(count);
  for (auto i = 0uz; i < count; ++i) {
    auto& p = ps.emplace_back();

    p.x = x + c->randxstart(),
    p.y = y + c->randystart(),
    // p.angle = c.randangle();
    // p.radius = c.randradius();
    p.vx = c->randxvel();
    p.vy = c->randyvel();
    p.gx = c->randgx();
    p.gy = c->randgy();
    p.av = c->randrotvel();
    p.af = c->randrotforce();
    p.life = c->randlife();
    p.alpha = c->randalpha();
    p.scale = c->randscale();
  }

  return std::make_shared<particlebatch>(c, ps);
}

particlesystem::particlesystem(std::shared_ptr<framework::resourcemanager> resourcemanager) noexcept
  : _factory(std::make_shared<particlefactory>(resourcemanager)) {
}

void graphics::particlesystem::add(const std::shared_ptr<particlebatch>& batch) noexcept {
  if (!batch) {
    return;
  }

  _batches.emplace_back(std::move(batch));
}

void graphics::particlesystem::set(const std::vector<std::shared_ptr<particlebatch>>& batches) noexcept {
  if (batches.empty()) {
    _batches.clear();
    return;
  }

  _batches = std::move(batches);
}

void graphics::particlesystem::clear() noexcept {
  _batches.clear();
}

void particlesystem::update(float_t delta) noexcept {
  for (const auto& batch : _batches) {
    auto& c = batch->conf;
    if (!c->active) [[unlikely]] {
      continue;
    }

    auto& ps = batch->particles;
    for (auto& p : ps) {
      p.life -= delta;

      if (p.life > 0.f) {
        p.av += p.af * delta;
        p.angle += p.av * delta;

        p.vx += p.gx * delta;
        p.vy += p.gy * delta;
        p.x += p.vx * delta;
        p.y += p.vy * delta;

        p.alpha = static_cast<uint8_t>(std::clamp(255.f * p.life, 0.f, 255.f));

        continue;
      }

      p.x = c->x + c->randxstart();
      p.y = c->y + c->randystart();
      p.vx = c->randxvel();
      p.vy = c->randyvel();
      p.gx = c->randgx();
      p.gy = c->randgy();
      p.av = c->randrotvel();
      p.af = c->randrotforce();
      p.scale = c->randscale();
      p.life = c->randlife();
      p.alpha = c->randalpha();
      p.scale = c->randscale();
    }
  }
}

void particlesystem::draw() const noexcept {
  for (const auto& batch : _batches) {
    auto& c = batch->conf;
    if (!c->active) [[unlikely]] {
      continue;
    }

    auto& ps = batch->particles;

    const auto& pixmap = *c->pixmap;
    const auto width = static_cast<float>(pixmap.width());
    const auto height = static_cast<float>(pixmap.height());
    const geometry::rectangle source{0, 0, width, height};

    for (const auto& p : ps) {
      const geometry::rectangle destination{p.x, p.y, width * p.scale, height * p.scale};

      pixmap.draw(
        source,
        destination,
        static_cast<double>(p.angle),
        static_cast<uint8_t>(p.alpha)
      );
    }
  }
}

std::shared_ptr<particlefactory> particlesystem::factory() const noexcept {
  return _factory;
}
