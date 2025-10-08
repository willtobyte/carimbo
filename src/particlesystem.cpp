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

  const auto count = j.value("count", 0ull);

  const auto& xvel = j.value("velocity", nlohmann::json::object()).value("x", nlohmann::json::object());
  const auto& yvel = j.value("velocity", nlohmann::json::object()).value("y", nlohmann::json::object());
  const auto& gx = j.value("gravity", nlohmann::json::object()).value("x", nlohmann::json::object());
  const auto& gy = j.value("gravity", nlohmann::json::object()).value("y", nlohmann::json::object());
  const auto& scale = j.value("scale", nlohmann::json::object());
  const auto& life = j.value("life", nlohmann::json::object());
  const auto& alpha = j.value("alpha", nlohmann::json::object());

  conf c{};
  c.x = x;
  c.y = y;
  c.pixmap = pixmap;
  c.xveldist = std::uniform_real_distribution<float>(xvel.value("start", .0f), xvel.value("end", .0f));
  c.yveldist = std::uniform_real_distribution<float>(yvel.value("start", .0f), yvel.value("end", .0f));
  c.gxdist = std::uniform_real_distribution<float>(gx.value("start", .0f), gx.value("end", .0f));
  c.gydist = std::uniform_real_distribution<float>(gy.value("start", .0f), gy.value("end", .0f));
  c.lifedist = std::uniform_real_distribution<float>(life.value("start", 1.0f), life.value("end", 1.0f));
  c.alphadist = std::uniform_int_distribution<unsigned int>(alpha.value("start", 255u), alpha.value("end", 255u));
  c.scaledist = std::uniform_real_distribution<float>(scale.value("start", 1.0f), scale.value("end", 1.0f));

  auto ps = std::vector<particle>();
  ps.reserve(count);
  for (auto i = 0uz; i < count; ++i) {
    auto& p = ps.emplace_back();

    p.angle = 0.0;
    p.x = x,
    p.y = y,
    p.vx = c.randxvel();
    p.vy = c.randyvel();
    p.gx = c.randgx();
    p.gy = c.randgy();
    p.life = c.randlife();
    p.alpha = c.randalpha();
    p.scale = c.randscale();
  }

  return std::make_shared<particlebatch>(c, ps);
}

particlesystem::particlesystem(std::shared_ptr<framework::resourcemanager> resourcemanager) noexcept
  : _factory(std::make_shared<particlefactory>(resourcemanager)) {
}

void graphics::particlesystem::add(std::shared_ptr<particlebatch> batch) noexcept {
  if (!batch) {
    return;
  }

  _batches.emplace_back(std::move(batch));
}

void graphics::particlesystem::set(std::vector<std::shared_ptr<particlebatch>> batches) noexcept {
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
    auto& ps = batch->particles;
    for (auto& p : ps) {
      p.life -= delta;

      if (p.life > 0.f) {
        p.vx += p.gx * delta;
        p.vy += p.gy * delta;
        p.x += p.vx * delta;
        p.y += p.vy * delta;
        p.alpha = static_cast<uint8_t>(std::clamp(255.f * p.life, 0.f, 255.f));

        continue;
      }

      p.angle = .0;
      p.x = c.x;
      p.y = c.y;
      p.vx = c.randxvel();
      p.vy = c.randyvel();
      p.gx = c.randgx();
      p.gy = c.randgy();
      p.scale = c.randscale();
      p.life = c.randlife();
      p.alpha = c.randalpha();
      p.scale = c.randscale();
    }
  }
}

void particlesystem::draw() const noexcept {
  for (const auto& batch : _batches) {
    auto& c = batch->conf;
    auto& ps = batch->particles;

    const auto& pixmap = *c.pixmap;
    const auto width = static_cast<float>(pixmap.width());
    const auto height = static_cast<float>(pixmap.height());
    const geometry::rectangle source{0, 0, width, height};

    for (const auto& p : ps) {
      const geometry::rectangle destination{p.x, p.y, width * p.scale, height * p.scale};

      pixmap.draw(
        source,
        destination,
        p.angle,
        p.alpha
      );
    }
  }
}

std::shared_ptr<particlefactory> particlesystem::factory() const noexcept {
  return _factory;
}
