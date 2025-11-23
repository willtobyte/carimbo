#include "particlesystem.hpp"

#include "io.hpp"
#include "pixmap.hpp"
#include "pixmappool.hpp"
#include "resourcemanager.hpp"

particlefactory::particlefactory(std::shared_ptr<resourcemanager> resourcemanager)
  : _resourcemanager(std::move(resourcemanager)) {
}

std::shared_ptr<particlebatch> particlefactory::create(std::string_view kind, float x, float y, bool emitting) const {
  const auto filename = std::format("particles/{}.json", kind);
  const auto buffer = io::read(filename);
  const auto j = nlohmann::json::parse(buffer);

  const auto pixmap = _resourcemanager->pixmappool()->get(std::format("blobs/particles/{}.png", kind));

  const auto count = j.value("count", 0uz);

  const auto spawn = j.value("spawn", nlohmann::json::object());
  const auto velocity = j.value("velocity", nlohmann::json::object());
  const auto gravity = j.value("gravity", nlohmann::json::object());
  const auto rotation = j.value("rotation", nlohmann::json::object());

  const auto radius = spawn.value("radius", nlohmann::json::object());
  const auto angle = spawn.value("angle", nlohmann::json::object());
  const auto xspawn = spawn.value("x", nlohmann::json::object());
  const auto yspawn = spawn.value("y", nlohmann::json::object());
  const auto scale = spawn.value("scale", nlohmann::json::object());
  const auto life = spawn.value("life", nlohmann::json::object());
  const auto alpha = spawn.value("alpha", nlohmann::json::object());
  const auto xvel = velocity.value("x", nlohmann::json::object());
  const auto yvel = velocity.value("y", nlohmann::json::object());
  const auto gx = gravity.value("x", nlohmann::json::object());
  const auto gy = gravity.value("y", nlohmann::json::object());
  const auto rforce = rotation.value("force", nlohmann::json::object());
  const auto rvel = rotation.value("velocity", nlohmann::json::object());

  const auto ps = std::make_shared<particleprops>();
  ps->active = true;
  ps->emitting = emitting;
  ps->x = x;
  ps->y = y;
  ps->pixmap = pixmap;
  ps->xspawnd = std::uniform_real_distribution<float>(xspawn.value("start", .0f), xspawn.value("end", .0f));
  ps->yspawnd = std::uniform_real_distribution<float>(yspawn.value("start", .0f), yspawn.value("end", .0f));
  ps->radiusd = std::uniform_real_distribution<float>(radius.value("start", .0f), radius.value("end", .0f));
  ps->angled = std::uniform_real_distribution<double>(angle.value("start", .0), angle.value("end", .0));
  ps->xveld = std::uniform_real_distribution<float>(xvel.value("start", .0f), xvel.value("end", .0f));
  ps->yveld = std::uniform_real_distribution<float>(yvel.value("start", .0f), yvel.value("end", .0f));
  ps->gxd = std::uniform_real_distribution<float>(gx.value("start", .0f), gx.value("end", .0f));
  ps->gyd = std::uniform_real_distribution<float>(gy.value("start", .0f), gy.value("end", .0f));
  ps->lifed = std::uniform_real_distribution<float>(life.value("start", 1.0f), life.value("end", 1.0f));
  ps->alphad = std::uniform_int_distribution<unsigned int>(alpha.value("start", 255u), alpha.value("end", 255u));
  ps->scaled = std::uniform_real_distribution<float>(scale.value("start", 1.0f), scale.value("end", 1.0f));
  ps->rotforced = std::uniform_real_distribution<double>(rforce.value("start", .0), rforce.value("end", .0));
  ps->rotveld = std::uniform_real_distribution<double>(rvel.value("start", .0),   rvel.value("end", .0));

  auto pb = std::make_shared<particlebatch>();
  pb->props = std::move(ps);
  pb->particles.resize(count);

  return pb;
}

particlesystem::particlesystem(std::shared_ptr<resourcemanager> resourcemanager)
  : _factory(std::make_shared<particlefactory>(resourcemanager)) {
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

    for (auto i = n; i-- > 0uz;) {
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

      if (!props->emitting) [[unlikely]] {
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
  for (const auto& batch : _batches) {
    auto& props = batch->props;
    if (!props->active) [[unlikely]] {
      continue;
    }

    const auto n = batch->size();
    const auto& pixmap = *props->pixmap;
    const auto w = static_cast<float>(pixmap.width());
    const auto h = static_cast<float>(pixmap.height());

    const auto* particles = batch->particles.data();

    for (auto i = n; i-- > 0uz;) {
      const auto& p = particles[i];

      pixmap.draw(
        0.f, 0.f, w, h,
        p.x, p.y, w * p.scale, h * p.scale,
        p.angle,
        p.alpha
      );
    }
  }
}

std::shared_ptr<particlefactory> particlesystem::factory() const {
  return _factory;
}
