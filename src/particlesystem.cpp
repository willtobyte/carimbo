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
   {
    "count": 100,
    "xvel": {"start": 1.0, "end": 1.0},
    "yvel": {"start": 1.0, "end": 1.0},
    "gx": {"start": 1.0, "end": 1.0},
    "gy": {"start": 1.0, "end": 1.0},
    "life": {"start": 1.0, "end": 1.0},
    ...
    ]
   }
   */
  const auto count = j.value("count", 0ull);

  const auto xvel = j["xvel"];
  const auto yvel = j["yvel"];
  const auto gx = j["gx"];
  const auto gy = j["gy"];
  const auto life = j["life"];
  const auto alpha = j["alpha"];

  emitter e{};
  e.x = x;
  e.y = y;
  e.pixmap = id;
  e.xveldist = std::uniform_real_distribution<float>(xvel.value("start", .0f), xvel.value("end", .0f));
  e.yveldist = std::uniform_real_distribution<float>(yvel.value("start", .0f), yvel.value("end", .0f));
  e.gxdist = std::uniform_real_distribution<float>(gx.value("start", .0f), gx.value("end", .0f));
  e.gydist = std::uniform_real_distribution<float>(gy.value("start", .0f), gy.value("end", .0f));
  e.lifedist = std::uniform_real_distribution<float>(life.value("start", .0f), life.value("end", .0f));
  e.alphadist = std::uniform_int_distribution<unsigned int>(alpha.value("start", 255u), alpha.value("end", 0u));

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
    p.alpha = e.randalpha();
    p.scale = 1.f;
    p.pixmap = e.pixmap;

    particles.emplace_back(p);
  }

  _pixmaps.try_emplace(id, std::move(pixmap));
  _emitters.try_emplace(name, std::move(e));
  _particles.try_emplace(name, std::move(particles));
}

void particlesystem::destroy(const std::string& name) noexcept {
  const auto pit = _particles.find(name);
  if (pit == _particles.end()) [[unlikely]] {
    return;
  }

  const auto eit = _emitters.find(name);
  if (eit != _emitters.end()) {
    _pixmaps.erase(eit->second.pixmap);
  }

  _emitters.erase(name);
  _particles.erase(pit);
}

void particlesystem::update(float_t delta) noexcept {
  for (auto& [name, particles] : _particles) {
    const auto eit = _emitters.find(name);
    if (eit == _emitters.end()) [[unlikely]] {
      return;
    }

    auto& e = eit->second;

    std::for_each(particles.begin(), particles.end(), [&](particle& p) {
      p.life -= delta;

      if (p.life > .0f) {
        p.vx += p.gx * delta;
        p.vy += p.gy * delta;
        p.x  += p.vx * delta;
        p.y  += p.vy * delta;
        p.alpha = static_cast<uint8_t>(std::clamp(255.f * p.life, 0.f, 255.f));

        return;
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
      p.pixmap = e.pixmap;
    });
  }
}

void particlesystem::draw() const noexcept {
  for (const auto& [name, particles] : _particles) {
    const auto eit = _emitters.find(name);
    if (eit == _emitters.end()) [[unlikely]] {
      continue;
    }

    const auto pit = _pixmaps.find(eit->second.pixmap);
    if (pit == _pixmaps.end()) [[unlikely]] {
      continue;
    }

    const auto& pixmap = *pit->second;
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
