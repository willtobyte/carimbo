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
  if (const auto it = std::find_if(_entities.begin(), _entities.end(), [&kind](const auto &e) {
        return e->kind() == kind;
      });
      it != _entities.end()) {
    return clone(*it);
  }

  const auto buffer = storage::io::read((std::ostringstream() << "entities/" << kind << ".json").str());
  const auto j = nlohmann::json::parse(buffer);

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
      0.,
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

std::shared_ptr<entity> entitymanager::clone(const std::shared_ptr<entity> &matrix) noexcept {
  if (!matrix) {
    return nullptr;
  }

  auto props = matrix->props();
  props.id = _counter++;
  props.frame = {};
  props.last_frame = SDL_GetTicks();
  props.angle = {};
  props.alpha = {255};
  props.position = {};
  props.pivot = {};
  props.velocity = {};
  props.action = {};
  props.reflection = {graphics::reflection::none};

  const auto e = entity::create(std::move(props));

  _entities.emplace_back(e);

  std::cout << "[entitymanager] cloned entity " << e->id() << " from matrix " << matrix->id() << std::endl;

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
  for (auto &entity : _entities) {
    entity->update(delta);
  }

  for (auto it = _entities.begin(); it != _entities.end(); ++it) {
    const auto &a = *it;
    for (const auto &b : std::ranges::subrange(std::next(it), _entities.end())) {
      if (!a->intersects(b))
        continue;

      const auto callback_a = get_or_default(a->_collisionmapping, b->kind(), noop_fn);
      const auto callback_b = get_or_default(b->_collisionmapping, a->kind(), noop_fn);

      callback_a(a, b);
      callback_b(b, a);

      SDL_Event event{};
      event.type = input::eventtype::collision;
      auto ptr = std::make_unique<collision>(a->id(), b->id());
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
