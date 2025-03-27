#include "entitymanager.hpp"
#include "common.hpp"
#include "entityprops.hpp"

using namespace framework;

template <typename T>
std::optional<std::function<T>> operator||(const std::optional<std::function<T>> &lhs, const std::optional<std::function<T>> &rhs) {
  return lhs ? lhs : rhs;
}

template <typename Map>
auto get_callback_or(const Map &m, const typename Map::key_type &key, std::optional<typename Map::mapped_type> fallback) {
  if (const auto it = m.find(key); it != m.end()) {
    return std::optional<typename Map::mapped_type>{it->second};
  }

  return fallback;
}

entitymanager::entitymanager(std::shared_ptr<resourcemanager> resourcemanager) noexcept
    : _resourcemanager(resourcemanager) {
}

std::shared_ptr<entity> entitymanager::spawn(const std::string &kind) {
  if (const auto it = std::ranges::find_if(_entities, [&kind](const auto &e) { return e->kind() == kind; }); it != _entities.end()) {
    return clone(*it);
  }

  const auto buffer = storage::io::read(fmt::format("entities/{}.json", kind));
  const auto j = nlohmann::json::parse(buffer);

  const auto scale = j.value("scale", float_t{1.f});

  auto spritesheet = j.contains("spritesheet")
                         ? _resourcemanager->pixmappool()->get(j["spritesheet"].get_ref<const std::string &>())
                         : nullptr;

  std::unordered_map<std::string, graphics::animation> animations;
  animations.reserve(j["animations"].size());

  for (const auto &item : j["animations"].items()) {
    const auto &key = item.key();
    const auto &a = item.value();

    const auto oneshot = a.value("oneshot", false);

    const auto hitbox = a.contains("hitbox")
                            ? std::make_optional(a["hitbox"].template get<geometry::rect>())
                            : std::nullopt;

    const auto &f = a["frames"];
    std::vector<graphics::keyframe> keyframes(f.size());
    std::ranges::transform(f, keyframes.begin(), [](const auto &frame) {
      return graphics::keyframe{
          frame["rect"].template get<geometry::rect>(),
          frame.value("offset", geometry::point{}),
          frame["duration"].template get<uint64_t>(),
      };
    });

    animations.emplace(key, graphics::animation{oneshot, hitbox, keyframes});
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
      scale,
      {},
      kind,
      "",
      graphics::reflection::none,
      spritesheet,
      animations,
  };

  auto e = entity::create(props);
  fmt::println("[entitymanager] spawn {} kind {}", e->id(), kind);
  _entities.emplace_back(e);
  return e;
}

std::shared_ptr<entity> entitymanager::clone(std::shared_ptr<entity> matrix) noexcept {
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

  const auto e = entity::create(props);

  _entities.emplace_back(e);

  fmt::println("[entitymanager] cloned entity {} from matrix {}", e->id(), matrix->id());

  return e;
}

void entitymanager::destroy(std::shared_ptr<entity> entity) noexcept {
  if (!entity) {
    return;
  }

  _entities.erase(std::remove(_entities.begin(), _entities.end(), entity), _entities.end());
}

std::shared_ptr<entity> entitymanager::find(uint64_t id) const noexcept {
  auto it = std::ranges::find_if(_entities, [id](const auto &entity) {
    return entity->id() == id;
  });

  return (it != _entities.end()) ? *it : nullptr;
}

void entitymanager::update(float_t delta) noexcept {
  std::for_each(
      _entities.begin(),
      _entities.end(),
      [delta](const auto &entity) {
        entity->update(delta);
      }
  );

  std::sort(_entities.begin(), _entities.end(), [](const auto &a, const auto &b) {
    return a->position().x() < b->position().x();
  });

  for (auto it = _entities.begin(); it != _entities.end(); ++it) {
    const auto &a = *it;
    const auto &props_a = a->props();

    const auto aita = props_a.animations.find(props_a.action);
    if (aita == props_a.animations.end() || !aita->second.hitbox) {
      continue;
    }

    const auto &ha = *aita->second.hitbox;

    const auto has = geometry::rect{
        a->position() + ha.position() * props_a.scale,
        ha.size() * props_a.scale
    };

    for (auto jt = std::next(it); jt != _entities.end(); ++jt) {
      const auto &b = *jt;

      const auto &props_b = b->props();
      const auto aitb = props_b.animations.find(props_b.action);
      if (aitb == props_b.animations.end() || !aitb->second.hitbox) {
        continue;
      }

      const auto &hb = *aitb->second.hitbox;

      const auto hbs = geometry::rect{
          b->position() + hb.position() * props_b.scale,
          hb.size() * props_b.scale
      };

      if (hbs.position().x() > has.position().x() + has.size().width()) break;
      if (hbs.position().y() > has.position().y() + has.size().height()) continue;
      if (hbs.position().y() + hbs.size().height() < has.position().y()) continue;

      if (!has.intersects(hbs)) {
        continue;
      }

      const auto callback_a = get_callback_or(a->_collisionmapping, b->kind(), std::nullopt);
      const auto callback_b = get_callback_or(b->_collisionmapping, a->kind(), std::nullopt);

      if (callback_a) (*callback_a)(a, b);
      if (callback_b) (*callback_b)(b, a);

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
