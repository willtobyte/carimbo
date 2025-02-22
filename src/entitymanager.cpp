#include "entitymanager.hpp"
#include "common.hpp"
#include "entityprops.hpp"
#include <ostream>

using namespace framework;

static void noop(const std::shared_ptr<entity> &, const std::shared_ptr<entity> &) noexcept {}

static const std::function<void(const std::shared_ptr<entity> &, const std::shared_ptr<entity> &)> noop_fn = noop;

template <typename T>
std::function<T> operator||(const std::function<T> &lhs, const std::function<T> &rhs) {
  return lhs ? lhs : rhs;
}

template <typename Map>
auto get_or_default(const Map &m, const typename Map::key_type &key, const typename Map::mapped_type &d) {
  auto it = m.find(key);
  return (it != m.end()) ? it->second : d;
}

entitymanager::entitymanager(std::shared_ptr<resourcemanager> resourcemanager) noexcept
    : _resourcemanager{std::move(resourcemanager)} {
}

std::shared_ptr<entity> entitymanager::spawn(const std::string &kind) {
  nlohmann::json j;
  if (const auto it = _cache.find(kind); it != _cache.end()) {
    j = it->second;
  } else {
    const auto buffer = storage::io::read((std::ostringstream() << "entities/" << kind << ".json").str());
    j = nlohmann::json::parse(buffer);
    _cache[kind] = j;
  }

  const auto size = j["size"].get<geometry::size>();
  const auto scale = j["scale"].get<float_t>();

  auto spritesheet = j.contains("spritesheet")
                         ? _resourcemanager->pixmappool()->get(j["spritesheet"].get_ref<const std::string &>())
                         : nullptr;

  std::map<std::string, animation> animations;
  for (const auto &[key, anim] : j["animations"].items()) {
    const auto hitbox = anim.contains("hitbox")
                            ? std::make_optional(anim["hitbox"].get<geometry::rect>())
                            : std::nullopt;

    std::vector<keyframe> keyframes;
    keyframes.reserve(16);
    for (const auto &frame : anim["frames"]) {
      keyframes.emplace_back(
          keyframe{
              frame["rect"].get<geometry::rect>(),
              frame.value("offset", geometry::point{}),
              frame["duration"].get<uint64_t>(),
              frame.value("singleshoot", false)
          }
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

void entitymanager::flush() noexcept {
  std::cout << "[entitymanager] flushing cache with " << _cache.size() << " entries" << std::endl;
  _cache.clear();
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

void entitymanager::update(float_t delta_time) noexcept {
  for (auto &entity : _entities) {
    entity->update(delta_time);
  }

  for (auto it = _entities.begin(); it != _entities.end(); ++it) {
    const auto &entity_a = *it;
    for (const auto &entity_b : std::ranges::subrange(std::next(it), _entities.end())) {
      if (!entity_a->intersects(entity_b))
        continue;

      const auto callback_a = get_or_default(entity_a->_collisionmapping, entity_b->kind(), noop_fn);
      const auto callback_b = get_or_default(entity_b->_collisionmapping, entity_a->kind(), noop_fn);

      callback_a(entity_a, entity_b);
      callback_b(entity_b, entity_a);

      SDL_Event event{};
      event.type = input::eventtype::collision;
      auto ptr = std::make_unique<collision>(entity_a->id(), entity_b->id());
      event.user.data1 = ptr.release();
      SDL_PushEvent(&event);
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
