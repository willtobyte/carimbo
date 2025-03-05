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
auto ensure_callback(const Map &m, const typename Map::key_type &key, const typename Map::mapped_type &d) {
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

  const auto buffer = storage::io::read(fmt::format("entities/{}.json", kind));
  const auto j = nlohmann::json::parse(buffer);

  const auto size = j["size"].get<geometry::size>();
  const auto scale = j["scale"].get<float_t>();

  auto spritesheet = j.contains("spritesheet")
                         ? _resourcemanager->pixmappool()->get(j["spritesheet"].get_ref<const std::string &>())
                         : nullptr;

  std::unordered_map<std::string, graphics::animation> animations;
  animations.reserve(j["animations"].size());
  for (const auto &[key, anim] : j["animations"].items()) {
    const auto hitbox = anim.contains("hitbox")
                            ? std::make_optional(anim["hitbox"].get<geometry::rect>())
                            : std::nullopt;

    std::vector<graphics::keyframe> keyframes;
    keyframes.reserve(anim["frames"].size());
    for (const auto &frame : anim["frames"]) {
      keyframes.emplace_back(
          graphics::keyframe{
              frame["rect"].get<geometry::rect>(),
              frame.value("offset", geometry::point{}),
              frame["duration"].get<uint64_t>(),
              frame.value("singleshoot", bool{false})
          }
      );
    }
    animations.emplace(key, graphics::animation{hitbox, std::move(keyframes)});
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
  fmt::println("[entitymanager] spawn {} kind {}", e->id(), kind);
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

  fmt::println("[entitymanager] cloned entity {} from matrix {}", e->id(), matrix->id());

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

      const auto callback_a = ensure_callback(a->_collisionmapping, b->kind(), noop_fn);
      const auto callback_b = ensure_callback(b->_collisionmapping, a->kind(), noop_fn);

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

void entitymanager::on_mousebuttondown(const input::mousebuttonevent &event) noexcept {
  if (event.button != input::mousebuttonevent::button::left) {
    return;
  }

  const geometry::point point{event.x, event.y};

  for (const auto &entity : _entities) {
    const auto &props = entity->props();
    if (props.action.empty()) {
      continue;
    }

    const auto it = props.animations.find(props.action);
    if (it == props.animations.end()) {
      continue;
    }

    const auto &animation = it->second;
    if (!animation.hitbox) {
      continue;
    }

    const auto hitbox = geometry::rect{
        entity->position() + animation.hitbox->position() * props.scale,
        animation.hitbox->size() * props.scale
    };

    if (!hitbox.contains(point)) {
      continue;
    }

    entity->on_touch();
  }
}
