#include "particlesystem.hpp"

using namespace graphics;

particlesystem::particlesystem(std::shared_ptr<framework::resourcemanager> resourcemanager) noexcept
    : _resourcemanager(std::move(resourcemanager)) {
}

void particlesystem::create(const std::string& name, const std::string& kind, float_t x, float_t y) {
  const auto& filename = std::format("particles/{}.json", kind);
  const auto& buffer = storage::io::read(filename);
  const auto& j = nlohmann::json::parse(buffer);

  const auto pixmap = _resourcemanager->pixmappool()->get(std::format("blobs/particles/{}.png", kind));
  const auto id = _counter++;

  /*
   [{
    "count": 100,
    "xvel": {"start": 1.0, "end": 1.0},
    "yvel": {"start": 1.0, "end": 1.0},
    "gx": {"start": 1.0, "end": 1.0},
    "gy": {"start": 1.0, "end": 1.0},
    "life": {"start": 1.0, "end": 1.0},
    "pixmaps": 3, // random between
    "distribuition": [10, 10, 80] // 3 pixmaps
    "width": 20,
    "height": 20,
   }]
   */
  const auto count = j.value("count", 0ull);

  const auto xvel = j["xvel"];
  const auto yvel = j["yvel"];
  const auto gx = j["gx"];
  const auto gy = j["gy"];
  const auto life = j["life"];

  emitter e{};
  e.x = x;
  e.y = y;
  e.pixmap = id;
  e.xveldist = std::uniform_real_distribution<float>(xvel.value("start", .0f), xvel.value("end", .0f));
  e.yveldist = std::uniform_real_distribution<float>(yvel.value("start", .0f), yvel.value("end", .0f));
  e.gxdist = std::uniform_real_distribution<float>(gx.value("start", .0f), gx.value("end", .0f));
  e.gydist = std::uniform_real_distribution<float>(gy.value("start", .0f), gy.value("end", .0f));
  e.lifedist = std::uniform_real_distribution<float>(life.value("start", .0f), life.value("end", .0f));

  auto particles = std::vector<particle>();
  particles.reserve(count);
  for (auto i = 0uz; i < count; ++i) {
    particle p{};
    p.angle = 0.0;
    p.x = e.x,
    p.y = e.y,
    p.vx = e.randxvel();
    p.vy = e.randyvel();
    p.gx = e.randgx();
    p.gy = e.randgy();
    p.life = e.randlife();
    p.pixmap = e.pixmap;
    p.alpha = 255;

    particles.emplace_back(p);
  }

  _pixmaps.try_emplace(id, std::move(pixmap));
  _emitters.try_emplace(name, std::move(e));
  _particles.try_emplace(name, std::move(particles));
}

void particlesystem::destroy(const std::string& name) noexcept {
  const auto it = _particles.find(name);
  if (it == _particles.end()) [[unlikely]] {
    return;
  }

  for (const auto& particle : it->second) {
    _pixmaps.erase(particle.pixmap);
  }

  _particles.erase(it);
}

void particlesystem::update(float_t delta) noexcept {
  for (auto& bucket : _particles) {
    auto& e = _emitters.find(bucket.first)->second;

    auto& particles = bucket.second;

    for (auto& p : particles) {
      p.life -= delta;

      if (p.life > .0f) {
        p.vx += p.gx * delta;
        p.vy += p.gy * delta;

        p.x += p.vx * delta;
        p.y += p.vy * delta;

        continue;
      }

      p.angle = 0.0;
      p.x = e.x;
      p.y = e.y;
      p.vx = e.randxvel();
      p.vy = e.randyvel();
      p.gx = e.randgx();
      p.gy = e.randgy();
      p.life = e.randlife();
      p.pixmap = e.pixmap;
      p.alpha = 255;
    }
  }
}

void particlesystem::draw() const noexcept {
  for (const auto& bucket : _particles) {
    const auto& particles = bucket.second;

    for (const auto& particle : particles) {
      const auto it = _pixmaps.find(particle.pixmap);
      if (it == _pixmaps.end()) [[unlikely]] {
        continue;
      }

      const auto& pixmap = *it->second;

      const auto width  = static_cast<float_t>(pixmap.width());
      const auto height = static_cast<float_t>(pixmap.height());
      const geometry::rectangle source{0, 0, width, height};

      const geometry::rectangle destination{
        particle.x,
        particle.y,
        width,
        height
      };

      pixmap.draw(
        source,
        destination,
        particle.angle,
        particle.alpha
      );
    }
  }
}
