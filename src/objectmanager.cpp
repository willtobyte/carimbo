#include "objectmanager.hpp"

using namespace framework;

using namespace input::event;

template <typename T>
std::optional<std::function<T>> operator||(const std::optional<std::function<T>>& lhs, const std::optional<std::function<T>>& rhs) {
  return lhs ? lhs : rhs;
}

template <typename Map>
auto get_callback_or(const Map& m, const typename Map::key_type& key, std::optional<typename Map::mapped_type> fallback) {
  if (const auto it = m.find(key); it != m.end()) {
    return std::optional<typename Map::mapped_type>{it->second};
  }

  return fallback;
}

objectmanager::objectmanager(std::shared_ptr<resourcemanager> resourcemanager)
    : _resourcemanager(resourcemanager),
      _collision_pool(collision_pool::instance()) {
  _collision_pool->reserve(1000);
}

std::shared_ptr<object> objectmanager::create(const std::string& kind, std::optional<std::reference_wrapper<const std::string>> scope, bool manage) {
  _dirty = true;

  const auto n = scope ? scope->get() : "";
  const auto& qualifier = n.empty() ? kind : fmt::format("{}/{}", n, kind);
  for (const auto& o : _objects) {
    if (o->_kind != kind) {
      continue;
    }

    if (n.empty()) {
      if (!o->_scope.empty()) {
        continue;
      }
    } else {
      if (o->_scope != n) {
        continue;
      }
    }

    return clone(o);
  }

  const auto& filename = fmt::format("objects/{}.json", qualifier);
  const auto& buffer = storage::io::read(filename);
  const auto& j = nlohmann::json::parse(buffer);

  const auto scale = j.value("scale", float_t{1.f});
  const auto spritesheet = _resourcemanager->pixmappool()->get(fmt::format("blobs/{}.png", qualifier));

  animation_map animations;
  animations.reserve(j["animations"].size());
  for (const auto& item : j["animations"].items()) {
    const auto& key = item.key();
    const auto& a = item.value();
    const auto oneshot = a.value("oneshot", false);
    const auto hitbox = a.contains("hitbox") ? std::make_optional(a.at("hitbox").template get<geometry::rectangle>()) : std::nullopt;
    const auto effect = a.contains("effect") ? _resourcemanager->soundmanager()->get(fmt::format("blobs/{}{}/{}.ogg", scope ? scope->get() : "", scope ? "/" : "", a.at("effect").template get_ref<const std::string&>())) : nullptr;
    const auto next = a.contains("next") ? std::make_optional(a.at("next").template get_ref<const std::string&>()) : std::nullopt;

    const auto& f = a.at("frames");
    std::vector<keyframe> keyframes(f.size());
    std::ranges::transform(f, keyframes.begin(), [](const auto& frame) {
      return keyframe{
          frame.at("rectangle").template get<geometry::rectangle>(),
          frame.at("offset").template get<geometry::point>(),
          frame.at("duration").template get<uint64_t>(),
      };
    });

    animations.emplace(key, animation{oneshot, next, hitbox, effect, keyframes});
  }

  auto o = std::make_shared<object>();
  o->_id = _counter++;
  o->_scale = scale;
  o->_kind = kind;
  o->_scope = n;
  o->_animations = std::move(animations);
  o->_spritesheet = std::move(spritesheet);

  fmt::println("[objectmanager] created {} {}", qualifier, o->id());
  if (manage) {
    _objects.emplace_back(o);
  }

  return o;
}

std::shared_ptr<object> objectmanager::clone(std::shared_ptr<object> matrix) {
  if (!matrix) [[unlikely]] {
    return nullptr;
  }

  const auto o = std::make_shared<object>();
  o->_id = _counter++;
  o->_angle = matrix->_angle;
  o->_kind = matrix->_kind;
  o->_scope = matrix->_scope;
  o->_action = matrix->_action;
  o->_spritesheet = matrix->_spritesheet;
  o->_animations = matrix->_animations;

  o->_position = matrix->_position;
  o->_velocity = matrix->_velocity;
  o->_scale = matrix->_scale;
  o->_reflection = matrix->_reflection;
  o->_alpha = matrix->_alpha;
  o->_hover = matrix->_hover;

  _objects.emplace_back(o);

  fmt::println("[objectmanager] clone {} from {}", matrix->id(), o->id());

  return o;
}

void objectmanager::manage(std::shared_ptr<object> object) {
  if (!object) {
    return;
  }

  _objects.emplace_back(std::move(object));
  _dirty = true;
}

void objectmanager::unmanage(std::shared_ptr<object> object) {
  if (!object) {
    return;
  }

  _objects.erase(
    std::remove(_objects.begin(), _objects.end(), object),
    _objects.end()
  );

  _dirty = true;
}

void objectmanager::destroy(std::shared_ptr<object> object) {
  if (!object) {
    return;
  }

  _dirty = true;
  _objects.erase(std::remove(_objects.begin(), _objects.end(), object), _objects.end());
  _objects.shrink_to_fit();
}

std::shared_ptr<object> objectmanager::find(uint64_t id) const {
  auto it = std::ranges::find_if(_objects, [id](const auto& object) {
    return object->id() == id;
  });

  return (it != _objects.end()) ? *it : nullptr;
}

void objectmanager::set_scenemanager(std::shared_ptr<scenemanager> scenemanager) {
  _scenemanager = std::move(scenemanager);
}

void objectmanager::update(float_t delta) noexcept {
  for (auto& o : _objects) {
    const auto old = o->x();

    o->update(delta);

    constexpr float_t epsilon = std::numeric_limits<float_t>::epsilon();
    if (std::abs(o->x() - old) > epsilon) {
      _dirty = true;
    }
  }

  for (auto it = _objects.begin(); it != _objects.end(); ++it) {
    const auto& a = *it;
    const auto aita = a->_animations.find(a->_action);
    if (aita == a->_animations.end() || !aita->second.hitbox) [[likely]] {
      continue;
    }

    const auto& ha = *aita->second.hitbox;
    const auto has = geometry::rectangle{a->position() + ha.position() * a->_scale, ha.size() * a->_scale};
    for (auto jt = std::next(it); jt != _objects.end(); ++jt) {
      const auto& b = *jt;

      const auto aitb = b->_animations.find(b->_action);
      if (aitb == b->_animations.end() || !aitb->second.hitbox) {
        continue;
      }

      const auto& hb = *aitb->second.hitbox;
      const auto hbs = geometry::rectangle{b->position() + hb.position() * b->_scale, hb.size() * b->_scale};
      if (hbs.x() > has.x() + has.width()) break;
      if (hbs.y() > has.y() + has.height()) continue;
      if (hbs.y() + hbs.height() < has.y()) continue;
      if (!has.intersects(hbs)) continue;

      const auto callback_a = get_callback_or(a->_collisionmapping, b->kind(), std::nullopt);
      const auto callback_b = get_callback_or(b->_collisionmapping, a->kind(), std::nullopt);

      if (callback_a) (*callback_a)(a, b);
      if (callback_b) (*callback_b)(b, a);

      SDL_Event event{};
      event.type = static_cast<uint32_t>(type::collision);
      auto o = _collision_pool->acquire(a->id(), b->id());
      event.user.data1 = o.release();

      SDL_PushEvent(&event);
    }
  }
}

void objectmanager::draw() const noexcept {
  for (const auto& o : _objects) {
    o->draw();
  }
}

void objectmanager::on_mouse_press(const mouse::button& event) {
  if (event.button != mouse::button::which::left) {
    return;
  }

  const geometry::point point{event.x, event.y};

  const auto clicked = std::ranges::any_of(_objects, [&](const auto& o) {
    if (o->_action.empty()) {
      return false;
    }

    const auto it = o->_animations.find(o->_action);
    if (it == o->_animations.end()) {
      return false;
    }

    const auto& animation = it->second;
    if (!animation.hitbox) {
      return false;
    }

    const auto hitbox = geometry::rectangle
    {
      o->_position + animation.hitbox->position() * o->_scale,
      animation.hitbox->size() * o->_scale
    };

    if (!hitbox.contains(point)) {
      return false;
    }

    o->on_touch(event.x, event.y);

    return true;
  });

  if (!clicked) {
    _scenemanager->on_touch(event.x, event.y);
  }
}

void objectmanager::on_mouse_motion(const input::event::mouse::motion& event) {
  for (const auto& o : _objects) {
    o->on_motion(event.x, event.y);
  }
}

void objectmanager::on_mail(const input::event::mail& event) {
  if (const auto& object = find(event.to); object) {
    object->on_email(event.body);
  }
}
