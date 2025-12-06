#include "particlesystem.hpp"

#include "io.hpp"
#include "pixmap.hpp"
#include "resourcemanager.hpp"

namespace {

template <typename T>
std::pair<T, T> read_range(unmarshal::object &object, T default_start, T default_end) noexcept {
  return {
    unmarshal::value_or(object, "start", default_start),
    unmarshal::value_or(object, "end", default_end)
  };
}

struct particleconfig final {
  size_t count{0};

  std::pair<float, float> xspawn{.0f, .0f};
  std::pair<float, float> yspawn{.0f, .0f};
  std::pair<float, float> radius{.0f, .0f};
  std::pair<double, double> angle{.0, .0};
  std::pair<float, float> scale{1.0f, 1.0f};
  std::pair<float, float> life{1.0f, 1.0f};
  std::pair<uint8_t, uint8_t> alpha{255u, 255u};

  std::pair<float, float> xvel{.0f, .0f};
  std::pair<float, float> yvel{.0f, .0f};

  std::pair<float, float> gx{.0f, .0f};
  std::pair<float, float> gy{.0f, .0f};

  std::pair<double, double> rforce{.0, .0};
  std::pair<double, double> rvel{.0, .0};

  friend void from_json(unmarshal::document &document, particleconfig &out) {
    for (auto field : document.get_object()) {
      const auto key = unmarshal::key(field);

      if (key == "count") {
        uint64_t value;
        if (!field.value().get_uint64().get(value)) {
          out.count = static_cast<size_t>(value);
        }
      } else if (key == "spawn") {
        auto object = unmarshal::object_of(field.value());
        for (auto spawn_field : object) {
          const auto spawn_key = unmarshal::key(spawn_field);
          auto spawn_object = unmarshal::object_of(spawn_field.value());

          if (spawn_key == "x") {
            out.xspawn = read_range(spawn_object, .0f, .0f);
          } else if (spawn_key == "y") {
            out.yspawn = read_range(spawn_object, .0f, .0f);
          } else if (spawn_key == "radius") {
            out.radius = read_range(spawn_object, .0f, .0f);
          } else if (spawn_key == "angle") {
            out.angle = read_range(spawn_object, .0, .0);
          } else if (spawn_key == "scale") {
            out.scale = read_range(spawn_object, 1.0f, 1.0f);
          } else if (spawn_key == "life") {
            out.life = read_range(spawn_object, 1.0f, 1.0f);
          } else if (spawn_key == "alpha") {
            out.alpha = read_range(spawn_object, 255u, 255u);
          }
        }
      } else if (key == "velocity") {
        auto object = unmarshal::object_of(field.value());
        for (auto vel_field : object) {
          const auto vel_key = unmarshal::key(vel_field);
          auto vel_object = unmarshal::object_of(vel_field.value());

          if (vel_key == "x") {
            out.xvel = read_range(vel_object, .0f, .0f);
          } else if (vel_key == "y") {
            out.yvel = read_range(vel_object, .0f, .0f);
          }
        }
      } else if (key == "gravity") {
        auto object = unmarshal::object_of(field.value());
        for (auto grav_field : object) {
          const auto grav_key = unmarshal::key(grav_field);
          auto grav_object = unmarshal::object_of(grav_field.value());

          if (grav_key == "x") {
            out.gx = read_range(grav_object, .0f, .0f);
          } else if (grav_key == "y") {
            out.gy = read_range(grav_object, .0f, .0f);
          }
        }
      } else if (key == "rotation") {
        auto object = unmarshal::object_of(field.value());
        for (auto rot_field : object) {
          const auto rot_key = unmarshal::key(rot_field);
          auto rot_object = unmarshal::object_of(rot_field.value());

          if (rot_key == "force") {
            out.rforce = read_range(rot_object, .0, .0);
          } else if (rot_key == "velocity") {
            out.rvel = read_range(rot_object, .0, .0);
          }
        }
      }
    }
  }
};
}

particlefactory::particlefactory(std::shared_ptr<resourcemanager> resourcemanager)
    : _resourcemanager(std::move(resourcemanager)) {
}

std::shared_ptr<particlebatch> particlefactory::create(std::string_view kind, float x, float y, bool spawning) const {
  const auto filename = std::format("particles/{}.json", kind);
  auto document = unmarshal::parse(io::read(filename));

  particleconfig conf;
  from_json(document, conf);

  const auto pixmap = _resourcemanager->pixmappool()->get(std::format("blobs/particles/{}.png", kind));

  const auto props = std::make_shared<particleprops>();
  props->active = true;
  props->spawning = spawning;
  props->x = x;
  props->y = y;
  props->pixmap = pixmap;
  props->xspawnd = std::uniform_real_distribution<float>(conf.xspawn.first, conf.xspawn.second);
  props->yspawnd = std::uniform_real_distribution<float>(conf.yspawn.first, conf.yspawn.second);
  props->radiusd = std::uniform_real_distribution<float>(conf.radius.first, conf.radius.second);
  props->angled = std::uniform_real_distribution<double>(conf.angle.first, conf.angle.second);
  props->xveld = std::uniform_real_distribution<float>(conf.xvel.first, conf.xvel.second);
  props->yveld = std::uniform_real_distribution<float>(conf.yvel.first, conf.yvel.second);
  props->gxd = std::uniform_real_distribution<float>(conf.gx.first, conf.gx.second);
  props->gyd = std::uniform_real_distribution<float>(conf.gy.first, conf.gy.second);
  props->lifed = std::uniform_real_distribution<float>(conf.life.first, conf.life.second);
  props->alphad = std::uniform_int_distribution<unsigned int>(conf.alpha.first, conf.alpha.second);
  props->scaled = std::uniform_real_distribution<float>(conf.scale.first, conf.scale.second);
  props->rotforced = std::uniform_real_distribution<double>(conf.rforce.first, conf.rforce.second);
  props->rotveld = std::uniform_real_distribution<double>(conf.rvel.first, conf.rvel.second);

  const auto batch = std::make_shared<particlebatch>();
  batch->props = std::move(props);
  batch->particles.resize(conf.count);
  batch->vertices.resize(conf.count * 4);

  batch->indices.resize(conf.count * 6);
  for (auto i = 0uz; i < conf.count; ++i) {
    const auto base = static_cast<int>(i * 4);
    const auto index = i * 6;
    batch->indices[index] = base;
    batch->indices[index + 1] = base + 1;
    batch->indices[index + 2] = base + 2;
    batch->indices[index + 3] = base;
    batch->indices[index + 4] = base + 2;
    batch->indices[index + 5] = base + 3;
  }

  return batch;
}

particlesystem::particlesystem(std::shared_ptr<resourcemanager> resourcemanager)
    : _renderer(resourcemanager->renderer()),
      _factory(std::make_shared<particlefactory>(resourcemanager)) {
  _batches.reserve(16);
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

    for (auto i = 0uz; i < n; ++i) {
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
  static constexpr auto u = std::array{.0f, 1.f, 1.f, .0f};
  static constexpr auto v = std::array{.0f, .0f, 1.f, 1.f};

  for (const auto& batch : _batches) {
    const auto& props = batch->props;
    if (!props->active) [[unlikely]] {
      continue;
    }

    const auto& pixmap = *props->pixmap;
    const auto hw = static_cast<float>(pixmap.width()) * 0.5f;
    const auto hh = static_cast<float>(pixmap.height()) * 0.5f;
    const auto n = batch->size();

    const auto* particles = batch->particles.data();
    auto* vertices = batch->vertices.data();

    for (auto i = 0uz; i < n; ++i) {
      const auto& p = particles[i];

      const auto shw = hw * p.scale;
      const auto shh = hh * p.scale;

      const auto ca = static_cast<float>(std::cos(p.angle));
      const auto sa = static_cast<float>(std::sin(p.angle));

      const auto lx = std::array{-shw, shw, shw, -shw};
      const auto ly = std::array{-shh, -shh, shh, shh};

      const SDL_FColor color = {1.f, 1.f, 1.f, p.alpha / 255.f};

      auto* vx = vertices + i * 4;

      for (auto j = 0uz; j < 4; ++j) {
        const auto rx = lx[j] * ca - ly[j] * sa;
        const auto ry = lx[j] * sa + ly[j] * ca;

        vx[j].position.x = p.x + rx;
        vx[j].position.y = p.y + ry;
        vx[j].tex_coord.x = u[j];
        vx[j].tex_coord.y = v[j];
        vx[j].color = color;
      }
    }

    SDL_RenderGeometry(
        *_renderer,
        static_cast<SDL_Texture*>(pixmap),
        batch->vertices.data(),
        static_cast<int>(batch->vertices.size()),
        batch->indices.data(),
        static_cast<int>(batch->indices.size())
    );
  }
}

std::shared_ptr<particlefactory> particlesystem::factory() const noexcept {
  return _factory;
}
