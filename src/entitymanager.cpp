#include "entitymanager.hpp"

#include "entity.hpp"
#include "io.hpp"
#include "resourcemanager.hpp"

using namespace framework;

using json = nlohmann::json;

void entitymanager::set_resourcemanager(
    std::shared_ptr<resourcemanager> resourcemanager) {
  _resourcemanager = resourcemanager;
}

std::shared_ptr<entity> entitymanager::spawn(const std::string_view id) {
  const auto buffer = storage::io::read(fmt::format("entities/{}.json", id));
  json j = buffer;

  std::cout << ">>> " << j["spritesheet"] << std::endl;
  /*
  namespace framework {
  struct keyframe {
    geometry::rect frame;
    uint64_t duration;

    keyframe() = default;
  };

  struct entityprops {
  public:
    std::shared_ptr<graphics::pixmap> spritesheet;
    std::map<std::string, std::vector<keyframe>> frames;
    geometry::point position;
    geometry::point pivot;
    float_t angle;
    graphics::flip flip;
    uint8_t alpha;
    std::string_view id;

    entityprops() = default;
  };
  }

  */
  const auto e = entity::create("abc");
  std::cout << "[entitymanager] spawn: " << e->id() << std::endl;
  _entities.emplace_back(e);
  return e;
}

void entitymanager::destroy(const std::shared_ptr<entity> entity) {
  _entities.remove(entity);
}

std::shared_ptr<entity> entitymanager::find(const std::string &id) {
  for (auto entity : _entities) {
    if (entity->id() == id) {
      return entity;
    }
  }

  return nullptr;
}

void entitymanager::update() {
  for (auto entity : _entities) {
    entity->update();
  }
}

void entitymanager::draw() {
  for (auto entity : _entities) {
    entity->draw();
  }
}
