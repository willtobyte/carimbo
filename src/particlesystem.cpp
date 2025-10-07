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

  const auto xvel = j["xvel"];
  const auto yvel = j["yvel"];
  const auto gx = j["gx"];
  const auto gy = j["gy"];
  const auto life = j["life"];
  const auto alpha = j["alpha"];

  emitter emitter{};
  emitter.x = x;
  emitter.y = y;
  emitter.pixmap = pixmap;
  emitter.xveldist = std::uniform_real_distribution<float>(xvel.value("start", .0f), xvel.value("end", .0f));
  emitter.yveldist = std::uniform_real_distribution<float>(yvel.value("start", .0f), yvel.value("end", .0f));
  emitter.gxdist = std::uniform_real_distribution<float>(gx.value("start", .0f), gx.value("end", .0f));
  emitter.gydist = std::uniform_real_distribution<float>(gy.value("start", .0f), gy.value("end", .0f));
  emitter.lifedist = std::uniform_real_distribution<float>(life.value("start", .0f), life.value("end", .0f));
  emitter.alphadist = std::uniform_int_distribution<unsigned int>(alpha.value("start", 255u), alpha.value("end", 0u));

  auto particles = std::vector<particle>();
  particles.reserve(count);
  for (auto i = 0uz; i < count; ++i) {
    auto& p = particles.emplace_back();

    p.angle = 0.0;
    p.x = emitter.x,
    p.y = emitter.y,
    p.vx = emitter.randxvel();
    p.vy = emitter.randyvel();
    p.gx = emitter.randgx();
    p.gy = emitter.randgy();
    p.life = emitter.randlife();
    p.alpha = emitter.randalpha();
    p.scale = 1.f;
  }

  return std::make_shared<particlebatch>(emitter, particles);
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
    auto& e = batch->emitter;
    auto& particles = batch->particles;
    for (auto& p : particles) {
      p.life -= delta;

      if (p.life > 0.f) {
        p.vx += p.gx * delta;
        p.vy += p.gy * delta;
        p.x  += p.vx * delta;
        p.y  += p.vy * delta;
        p.alpha = static_cast<uint8_t>(std::clamp(255.f * p.life, 0.f, 255.f));

        continue;
      }

      p.angle = .0;
      p.x = e.x;
      p.y = e.y;
      p.vx = e.randxvel();
      p.vy = e.randyvel();
      p.gx = e.randgx();
      p.gy = e.randgy();
      p.life = e.randlife();
      p.alpha = e.randalpha();
      p.scale = 1.f;
    }
  }
}

void particlesystem::draw() const noexcept {
  for (const auto& batch : _batches) {
    auto& e = batch->emitter;
    auto& particles = batch->particles;

    const auto& pixmap = *e.pixmap;
    const auto width = static_cast<float>(pixmap.width());
    const auto height = static_cast<float>(pixmap.height());
    const geometry::rectangle source{0, 0, width, height};

    for (const auto& p : particles) {
      const geometry::rectangle destination{p.x, p.y, width, height};

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
