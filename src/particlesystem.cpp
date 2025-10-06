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
  _pixmaps.try_emplace(id, std::move(pixmap));

  UNUSED(name);

  const auto count = 1000ull;

  auto particles = std::vector<particle>();
  particles.reserve(count);

  std::mt19937 rng{std::random_device{}()};
  std::uniform_real_distribution<float> xveldist(j["xvel"].value("start", .0f), j["xveldist"].value("end", .0f));
  std::uniform_real_distribution<float> yveldist(j["yvel"].value("start", .0f), j["yveldist"].value("end", .0f));
  std::uniform_real_distribution<float> gxdist(j["gx"].value("start", .0f), j["gx"].value("end", .0f));
  std::uniform_real_distribution<float> gydist(j["gy"].value("start", .0f), j["gy"].value("end", .0f));
  std::uniform_real_distribution<float> lifedist(j["life"].value("start", .0f), j["life"].value("end", .0f));

  for (auto i = 0uz; i < count; ++i) {
    particle p{};
    p.angle = 0.0;
    p.x = x,
    p.y = y,
    p.vx = xveldist(rng);
    p.vy = yveldist(rng);
    p.gx = gxdist(rng);
    p.gy = gydist(rng);
    p.life = lifedist(rng);
    p.frame = 0;
    p.pixmap = 0;
    p.alpha = 255;

    particles.emplace_back(p);
  }

  /*
  {
    "x"
    "y"
    "pix"
  }
  */
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
    auto& particles = bucket.second;

    for (auto& particle : particles) {
      particle.life -= delta;
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
