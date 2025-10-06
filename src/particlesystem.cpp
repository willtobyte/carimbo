#include "particlesystem.hpp"

using namespace graphics;

particlesystem::particlesystem(std::shared_ptr<framework::resourcemanager> resourcemanager) noexcept
    : _resourcemanager(std::move(resourcemanager)) {
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
