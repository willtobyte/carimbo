#include "objectmanager.hpp"

#include "objectpool.hpp"

using namespace framework;

using namespace input::event;

template <typename T>
std::optional<std::function<T>> operator||(const std::optional<std::function<T>>& lhs, const std::optional<std::function<T>>& rhs) {
  return lhs ? lhs : rhs;
}

template <typename Map>
auto callback_or(const Map& m, const typename Map::key_type& key, std::optional<typename Map::mapped_type> fallback) {
  if (const auto it = m.find(key); it != m.end()) {
    return std::optional<typename Map::mapped_type>{it->second};
  }

  return fallback;
}

objectmanager::objectmanager(std::shared_ptr<resourcemanager> resourcemanager)
    : _resourcemanager(resourcemanager),
      _objectpool(objectpool::instance()),
      _envelopepool(envelopepool::instance()) {
  _objectpool->reserve(1000);
  _envelopepool->reserve(1000);
}

std::shared_ptr<object> objectmanager::create(const std::string& kind, std::optional<std::reference_wrapper<const std::string>> scope, bool manage) {
  const auto n = scope ? scope->get() : "";
  const auto& qualifier = n.empty() ? kind : std::format("{}/{}", n, kind);
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

  const auto& filename = std::format("objects/{}.json", qualifier);
  const auto& buffer = storage::io::read(filename);
  const auto& j = nlohmann::json::parse(buffer);

  const auto scale = j.value("scale", float_t{1.f});
  const auto spritesheet = _resourcemanager->pixmappool()->get(std::format("blobs/{}.png", qualifier));

  animation_map animations;
  animations.reserve(j["animations"].size());
  for (const auto& item : j["animations"].items()) {
    const auto& key = item.key();
    const auto& a = item.value();
    const auto hitbox = a.contains("hitbox") ? std::make_optional(a.at("hitbox").template get<framework::hitbox>()) : std::nullopt;
    const auto effect = a.contains("effect")
        ? _resourcemanager->soundmanager()->get(
            scope
                ? std::format("blobs/{}/{}.ogg", scope->get(), a.at("effect").template get_ref<const std::string&>())
                : std::format("blobs/{}.ogg", a.at("effect").template get_ref<const std::string&>())
          ) : nullptr;
    const auto next = a.contains("next") ? std::make_optional(a.at("next").template get_ref<const std::string&>()) : std::nullopt;
    const bool oneshot = next.has_value() || a.value("oneshot", false);
    const auto keyframes = a.at("frames").get<std::vector<framework::keyframe>>();

    animations.emplace(key, animation{oneshot, next, hitbox, effect, std::move(keyframes)});
  }

  auto o = _objectpool->acquire();
  o->_id = _counter++;
  o->_scale = scale;
  o->_kind = kind;
  o->_scope = n;
  o->_animations = std::move(animations);
  o->_spritesheet = std::move(spritesheet);

  std::println("[objectmanager] created {} {}", qualifier, o->id());
  if (manage) {
    _objects.emplace_back(o);
  }

  return o;
}

std::shared_ptr<object> objectmanager::clone(std::shared_ptr<object> matrix) {
  if (!matrix) [[unlikely]] {
    return nullptr;
  }

  const auto o = _objectpool->acquire();
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

  std::println("[objectmanager] clone {} from {}", o->id(), matrix->id());

  return o;
}

void objectmanager::manage(std::shared_ptr<object> object) {
  if (!object) {
    return;
  }

  _objects.emplace_back(std::move(object));
}

void objectmanager::unmanage(std::shared_ptr<object> object) {
  if (!object) {
    return;
  }

  _objects.erase(
    std::remove(_objects.begin(), _objects.end(), object),
    _objects.end()
  );
}

void objectmanager::destroy(std::shared_ptr<object> object) {
  if (!object) {
    return;
  }

  _objects.erase(std::remove(_objects.begin(), _objects.end(), object), _objects.end());
  _objects.shrink_to_fit();

  _objectpool->release(object);
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

void objectmanager::update(float_t dt) noexcept {
  for (auto itoa = _objects.begin(); itoa != _objects.end(); ++itoa) {
    auto& a = *itoa;
    a->update(dt);

    const auto ita = a->_animations.find(a->_action);
    if (ita == a->_animations.end() || !ita->second.hitbox) {
      continue;
    }

    const auto& ha = *ita->second.hitbox;
    const auto ra = geometry::rectangle(
      a->position() + ha.rectangle.position() * a->_scale,
      ha.rectangle.size() * a->_scale
    );

    for (auto itob = std::next(itoa); itob != _objects.end(); ++itob) {
      auto& b = *itob;

      const auto itb = b->_animations.find(b->_action);
      if (itb == b->_animations.end() || !itb->second.hitbox) {
        continue;
      }

      const auto& hb = *itb->second.hitbox;

      const bool react =
        (!ha.type || hb.reagents.test(ha.type.value())) ||
        (!hb.type || ha.reagents.test(hb.type.value()));

      if (!react) {
        continue;
      }

      const auto rb = geometry::rectangle(
        b->position() + hb.rectangle.position() * b->_scale,
        hb.rectangle.size() * b->_scale
      );

      if (!ra.intersects(rb)) {
        continue;
      }

      if (const auto& cb = callback_or(a->_collisionmapping, b->kind(), std::nullopt); cb) {
        (*cb)(a, b);
      }

      if (const auto& cb = callback_or(b->_collisionmapping, a->kind(), std::nullopt); cb) {
        (*cb)(b, a);
      }

      SDL_Event event{};
      event.type = static_cast<uint32_t>(type::collision);
      event.user.data1 = _envelopepool->acquire(collisionenvelope(a->id(), b->id())).release();
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
      o->_position + animation.hitbox->rectangle.position() * o->_scale,
      animation.hitbox->rectangle.size() * o->_scale
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
