#include "particlesystem.hpp"

using namespace graphics;

particlesystem::particlesystem(std::shared_ptr<framework::resourcemanager> resourcemanager) noexcept
    : _resourcemanager(std::move(resourcemanager)) {
}

void particlesystem::create(const std::string& name, const std::string& kind) {
  const auto& filename = std::format("particles/{}.json", kind);
  const auto& buffer = storage::io::read(filename);
  const auto& j = nlohmann::json::parse(buffer);

  const auto spritesheet = _resourcemanager->pixmappool()->get(std::format("blobs/particles/{}.png", kind));
  const auto id = _counter++;
  _spritesheets.try_emplace(id, std::move(spritesheet));

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
    _spritesheets.erase(particle.pixmap);
  }

  _particles.erase(it);
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
