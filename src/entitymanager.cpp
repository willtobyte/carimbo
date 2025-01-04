#include "entitymanager.hpp"
#include "common.hpp"
#include "entityprops.hpp"
#include <ostream>

using namespace framework;

using json = nlohmann::json;

entitymanager::entitymanager(std::shared_ptr<resourcemanager> resourcemanager) noexcept
    : _resourcemanager{std::move(resourcemanager)} {
}

std::shared_ptr<entity> entitymanager::spawn(const std::string &kind) {
  const auto buffer = storage::io::read((std::ostringstream() << "entities/" << kind << ".json").str());
  const auto j = json::parse(buffer);

  const auto size = j["size"].get<geometry::size>();
  const auto scale = j["scale"].get<float_t>();

  auto spritesheet = j.contains("spritesheet")
                         ? _resourcemanager->pixmappool()->get(j["spritesheet"].get_ref<const std::string &>())
                         : nullptr;

  std::map<std::string, animation> animations;
  for (const auto &[key, anim] : j["animations"].items()) {
    const auto hitbox = anim.value("hitbox", geometry::rect{});

    std::vector<keyframe> keyframes;
    keyframes.reserve(16);
    for (const auto &frame : anim["frames"]) {
      keyframes.emplace_back(
          frame["rect"].get<geometry::rect>(),
          frame["duration"].get<uint64_t>(),
          frame.value("singleshoot", false),
          frame.value("offset", geometry::point{})
      );
    }
    animations.emplace(key, animation{hitbox, std::move(keyframes)});
  }

  entityprops props{
      _counter++,
      0,
      SDL_GetTicks(),
      0.0,
      255,
      true,
      {},
      {},
      size,
      scale,
      {},
      kind,
      "",
      graphics::reflection::none,
      std::move(spritesheet),
      std::move(animations),
  };

  auto e = entity::create(std::move(props));
  std::cout << "[entitymanager] spawn " << e->id() << " kind " << kind << std::endl;
  _entities.emplace_back(e);
  return e;
}

void entitymanager::destroy(const std::shared_ptr<entity> entity) noexcept {
  if (!entity) {
    return;
  }

  _entities.remove(entity);
}

std::shared_ptr<entity> entitymanager::find(uint64_t id) const noexcept {
  auto it = std::ranges::find_if(_entities, [id](const auto &entity) {
    return entity->id() == id;
  });

  return (it != _entities.end()) ? *it : nullptr;
}

void entitymanager::update(float_t delta) noexcept {
  for (const auto &entity : _entities) {
    entity->update(delta);
  }

  std::unordered_map<std::string, std::vector<std::shared_ptr<entity>>> mapping;
  for (const auto &entity : _entities) {
    mapping[entity->kind()].emplace_back(entity);
  }

  for (const auto &entity1 : _entities) {
    if (entity1->_collisionmapping.empty()) [[likely]] {
      continue;
    }

    const auto &pos1 = entity1->position();
    const auto &size1 = entity1->size() * entity1->props().scale;

    for (const auto &[kind, callback] : entity1->_collisionmapping) {
      if (auto it = mapping.find(kind); it != mapping.end()) [[likely]] {
        const auto &mapping = it->second;

        for (const auto &entity2 : mapping) {
          if (entity1 == entity2) [[unlikely]]
            continue;

          const auto &pos2 = entity2->position();
          const auto &size2 = entity2->size() * entity2->props().scale;

          if (pos1.x() < pos2.x() + size2.width() &&
              pos1.x() + size1.width() > pos2.x() &&
              pos1.y() < pos2.y() + size2.height() &&
              pos1.y() + size1.height() > pos2.y()) [[likely]] {

            callback(entity1, entity2->id());
          }
        }
      }
    }
  }
}

void entitymanager::draw() noexcept {
  for (const auto &entity : _entities) {
    entity->draw();
  }
}

void entitymanager::on_mail(const input::mailevent &event) noexcept {
  if (const auto entity = find(event.to); entity) {
    entity->on_email(event.body);
  }
}
