#include "particlesystem.hpp"

using namespace graphics;

void particlesystem::draw() const noexcept {
  for (const auto& bucket : _particles) {
    const auto& pixmap = _pixmaps.at(bucket.first);
    const auto width = static_cast<float_t>(pixmap->width());
    const auto height = static_cast<float_t>(pixmap->height());
    const geometry::rectangle source{0, 0, width, height};

    const auto& particles = bucket.second;
    for (const auto& particle : particles) {
      const geometry::rectangle destination{
        particle.x,
        particle.y,
        width,
        height
      };

      pixmap->draw(
        source,
        destination,
        particle.angle,
        particle.alpha
      );
    }
  }
}
