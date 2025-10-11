#include "particlesystem.hpp"

using namespace graphics;

particlefactory::particlefactory(std::shared_ptr<framework::resourcemanager> resourcemanager) noexcept
  : _resourcemanager(std::move(resourcemanager)) {
}

std::shared_ptr<particlebatch> particlefactory::create(const std::string& kind, float x, float y) const {
  const auto& filename = std::format("particles/{}.json", kind);
  const auto& buffer = storage::io::read(filename);
  const auto& j = nlohmann::json::parse(buffer);

  const auto pixmap = _resourcemanager->pixmappool()->get(std::format("blobs/particles/{}.png", kind));

  const auto count = j.value("count", 0uz);

  const auto& spawn = j.value("spawn", nlohmann::json::object());
  const auto& velocity = j.value("velocity", nlohmann::json::object());
  const auto& gravity = j.value("gravity", nlohmann::json::object());
  const auto& rotation = j.value("rotation", nlohmann::json::object());

  const auto& radius = spawn.value("radius", nlohmann::json::object());
  const auto& angle = spawn.value("angle", nlohmann::json::object());
  const auto& xspawn = spawn.value("x", nlohmann::json::object());
  const auto& yspawn = spawn.value("y", nlohmann::json::object());
  const auto& scale = spawn.value("scale", nlohmann::json::object());
  const auto& life = spawn.value("life", nlohmann::json::object());
  const auto& alpha = spawn.value("alpha", nlohmann::json::object());
  const auto& xvel = velocity.value("x", nlohmann::json::object());
  const auto& yvel = velocity.value("y", nlohmann::json::object());
  const auto& gx = gravity.value("x", nlohmann::json::object());
  const auto& gy = gravity.value("y", nlohmann::json::object());
  const auto& rforce = rotation.value("force", nlohmann::json::object());
  const auto& rvel = rotation.value("velocity", nlohmann::json::object());

  const auto conf = std::make_shared<particleconf>();
  conf->active = true;
  conf->x = x;
  conf->y = y;
  conf->pixmap = pixmap;
  conf->xspawnd = std::uniform_real_distribution<float>(xspawn.value("start", .0f), xspawn.value("end", .0f));
  conf->yspawnd = std::uniform_real_distribution<float>(yspawn.value("start", .0f), yspawn.value("end", .0f));
  conf->radiusd = std::uniform_real_distribution<float>(radius.value("start", .0f), radius.value("end", .0f));
  conf->angled = std::uniform_real_distribution<double>(angle.value("start", .0), angle.value("end", .0));
  conf->xveld = std::uniform_real_distribution<float>(xvel.value("start", .0f), xvel.value("end", .0f));
  conf->yveld = std::uniform_real_distribution<float>(yvel.value("start", .0f), yvel.value("end", .0f));
  conf->gxd = std::uniform_real_distribution<float>(gx.value("start", .0f), gx.value("end", .0f));
  conf->gyd = std::uniform_real_distribution<float>(gy.value("start", .0f), gy.value("end", .0f));
  conf->lifed = std::uniform_real_distribution<float>(life.value("start", 1.0f), life.value("end", 1.0f));
  conf->alphad = std::uniform_int_distribution<unsigned int>(alpha.value("start", 255u), alpha.value("end", 255u));
  conf->scaled = std::uniform_real_distribution<float>(scale.value("start", 1.0f), scale.value("end", 1.0f));
  conf->rotforced = std::uniform_real_distribution<double>(rforce.value("start", .0), rforce.value("end", .0));
  conf->rotveld = std::uniform_real_distribution<double>(rvel.value("start", .0),   rvel.value("end", .0));

  auto pb = std::make_shared<particlebatch>();
  pb->conf = std::move(conf);
  pb->x.resize(count);
  pb->y.resize(count);
  pb->vx.resize(count);
  pb->vy.resize(count);
  pb->gx.resize(count);
  pb->gy.resize(count);
  pb->av.resize(count);
  pb->af.resize(count);
  pb->life.resize(count);
  pb->scale.resize(count);
  pb->angle.resize(count);
  pb->alpha.resize(count);

  for (auto i = count; i-- > 0uz;) {
    pb->x[i] = x + conf->randxspawn();
    pb->y[i] = y + conf->randyspawn();
    pb->vx[i] = conf->randxvel();
    pb->vy[i] = conf->randyvel();
    pb->gx[i] = conf->randgx();
    pb->gy[i] = conf->randgy();
    pb->av[i] = conf->randrotvel();
    pb->af[i] = conf->randrotforce();
    pb->life[i] = conf->randlife();
    pb->alpha[i] = conf->randalpha();
    pb->scale[i] = conf->randscale();
    pb->angle[i] = conf->randangle();
  }

  return pb;
}

particlesystem::particlesystem(std::shared_ptr<framework::resourcemanager> resourcemanager) noexcept
  : _factory(std::make_shared<particlefactory>(resourcemanager)) {
  _batches.reserve(16);
}

void graphics::particlesystem::add(const std::shared_ptr<particlebatch>& batch) noexcept {
  if (!batch) [[unlikely]] {
    return;
  }

  _batches.emplace_back(batch);
}

void graphics::particlesystem::set(const std::vector<std::shared_ptr<particlebatch>>& batches) noexcept {
  if (batches.empty()) {
    _batches.clear();
    return;
  }

  _batches = batches;
}

void graphics::particlesystem::clear() noexcept {
  _batches.clear();
}

void particlesystem::update(float delta) noexcept {
  for (const auto& batch : _batches) {
    auto& conf = batch->conf;
    if (!conf->active) [[unlikely]] {
      continue;
    }

    const auto n = batch->size();
    for (auto i = n; i-- > 0uz;) {
      batch->life[i] -= delta;
      if (batch->life[i] > 0.f) {
        const auto d = static_cast<double>(delta);
        batch->av[i] += batch->af[i] * d;
        batch->angle[i] += batch->av[i] * d;

        batch->vx[i] += batch->gx[i] * delta;
        batch->vy[i] += batch->gy[i] * delta;
        batch->x[i] += batch->vx[i] * delta;
        batch->y[i] += batch->vy[i] * delta;

        const auto a = 255.f * batch->life[i];
        batch->alpha[i] = static_cast<std::uint8_t>(std::clamp(a, .0f, 255.f));
        continue;
      }

      batch->x[i] = conf->x + conf->randxspawn();
      batch->y[i] = conf->y + conf->randyspawn();
      batch->vx[i] = conf->randxvel();
      batch->vy[i] = conf->randyvel();
      batch->gx[i] = conf->randgx();
      batch->gy[i] = conf->randgy();
      batch->av[i] = conf->randrotvel();
      batch->af[i] = conf->randrotforce();
      batch->life[i] = conf->randlife();
      batch->alpha[i] = conf->randalpha();
      batch->scale[i] = conf->randscale();
      batch->angle[i] = conf->randangle();
    }
  }
}

void particlesystem::draw() const noexcept {
  for (const auto& batch : _batches) {
    auto& c = batch->conf;
    if (!c->active) [[unlikely]] {
      return;
    }

    const auto n = batch->size();
    const auto& pixmap = *c->pixmap;
    const auto width = static_cast<float>(pixmap.width());
    const auto height = static_cast<float>(pixmap.height());
    const geometry::rectangle source{0, 0, width, height};

    const auto* x = batch->x.data();
    const auto* y = batch->y.data();
    const auto* scale = batch->scale.data();
    const auto* angle = batch->angle.data();
    const auto* alpha = batch->alpha.data();

    for (auto i = n; i-- > 0uz;) {
      const geometry::rectangle destination{
        x[i], y[i],
        width  * scale[i],
        height * scale[i]
      };

      pixmap.draw(
        source,
        destination,
        angle[i],
        alpha[i]
      );
    }
  }
}

std::shared_ptr<particlefactory> particlesystem::factory() const noexcept {
  return _factory;
}
