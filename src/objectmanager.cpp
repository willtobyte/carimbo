#include "objectmanager.hpp"

using namespace framework;

using namespace input::event;

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

objectmanager::objectmanager(std::shared_ptr<resourcemanager> resourcemanager)
    : _resourcemanager(resourcemanager) {
}

std::shared_ptr<object> objectmanager::create(const std::string &kind, std::optional<std::reference_wrapper<const std::string>> scope, bool manage) {
  _dirty = true;

  const auto scoped = scope ? scope->get() : "";
  const auto &qualifier = scoped.empty() ? kind : fmt::format("{}/{}", scoped, kind);

  for (const auto& o : _objects) {
    const auto& p = o->props();

    if (p.kind != kind) continue;

    if (scoped.empty()) {
      if (!p.scope.empty()) continue;
    } else {
      if (p.scope != scoped) continue;
    }

    return clone(o);
  }

  const auto &filename = fmt::format("objects/{}.json", qualifier);
  const auto &buffer = storage::io::read(filename);
  const auto &j = nlohmann::json::parse(buffer);

  const auto scale = j.value("scale", float_t{1.f});
  const auto spritesheet = _resourcemanager->pixmappool()->get(fmt::format("blobs/{}.png", qualifier));

  #ifdef EMSCRIPTEN
  std::unordered_map<std::string, graphics::animation> animations;
  #else
  absl::flat_hash_map<std::string, graphics::animation> animations;
  #endif

  animations.reserve(j["animations"].size());

  for (const auto &item : j["animations"].items()) {
    const auto &key = item.key();
    const auto &a = item.value();
    const auto oneshot = a.value("oneshot", false);
    const auto hitbox = a.contains("hitbox") ? std::make_optional(a.at("hitbox").template get<geometry::rectangle>()) : std::nullopt;
    const auto effect = a.contains("effect") ? _resourcemanager->soundmanager()->get(fmt::format("blobs/{}{}/{}.ogg", scope ? scope->get() : "", scope ? "/" : "", a.at("effect").template get_ref<const std::string&>())) : nullptr;
    const auto next = a.contains("next") ? std::make_optional(a.at("next").template get_ref<const std::string&>()) : std::nullopt;

    const auto &f = a.at("frames");
    std::vector<graphics::keyframe> keyframes(f.size());
    std::ranges::transform(f, keyframes.begin(), [](const auto &frame) {
      return graphics::keyframe{
          frame.at("rectangle").template get<geometry::rectangle>(),
          frame.at("offset").template get<geometry::point>(),
          frame.at("duration").template get<uint64_t>(),
      };
    });

    animations.emplace(key, graphics::animation{oneshot, next, hitbox, effect, keyframes});
  }

  objectprops props{
      _counter++,
      0,
      SDL_GetTicks(),
      0.,
      255,
      {},
      scale,
      {},
      kind,
      scoped,
      "",
      false,
      graphics::reflection::none,
      std::move(spritesheet),
      std::move(animations),
  };

  auto o = std::make_shared<object>(props);
  fmt::println("[objectmanager] created {} {}", qualifier, o->id());
  if (manage) {
    _objects.emplace_back(o);
  }

  return o;
}

std::shared_ptr<object> objectmanager::clone(std::shared_ptr<object> matrix) {
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
  props.velocity = {};
  props.action = {};
  props.reflection = {graphics::reflection::none};

  const auto o = std::make_shared<object>(props);

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
  auto it = std::ranges::find_if(_objects, [id](const auto &object) {
    return object->id() == id;
  });

  return (it != _objects.end()) ? *it : nullptr;
}

void objectmanager::set_scenemanager(std::shared_ptr<scenemanager> scenemanager) {
  _scenemanager = std::move(scenemanager);
}

void objectmanager::update(float_t delta) {
  for (auto &o : _objects) {
    const auto old = o->x();

    o->update(delta);

    constexpr float_t epsilon = std::numeric_limits<float_t>::epsilon();
    if (std::abs(o->x() - old) > epsilon) {
      _dirty = true;
    }
  }

  for (auto it = _objects.begin(); it != _objects.end(); ++it) {
    const auto &a = *it;
    const auto &ap = a->props();

    const auto aita = ap.animations.find(ap.action);
    if (aita == ap.animations.end() || !aita->second.hitbox) [[likely]] {
      continue;
    }

    const auto &ha = *aita->second.hitbox;
    const auto has = geometry::rectangle{a->position() + ha.position() * ap.scale, ha.size() * ap.scale};
    for (auto jt = std::next(it); jt != _objects.end(); ++jt) {
      const auto &b = *jt;

      const auto &bp = b->props();
      const auto aitb = bp.animations.find(bp.action);
      if (aitb == bp.animations.end() || !aitb->second.hitbox) {
        continue;
      }

      const auto &hb = *aitb->second.hitbox;
      const auto hbs = geometry::rectangle{b->position() + hb.position() * bp.scale, hb.size() * bp.scale};
      if (hbs.position().x() > has.position().x() + has.size().width()) break;
      if (hbs.position().y() > has.position().y() + has.size().height()) continue;
      if (hbs.position().y() + hbs.size().height() < has.position().y()) continue;
      if (!has.intersects(hbs)) continue;

      const auto callback_a = get_callback_or(a->_collisionmapping, b->kind(), std::nullopt);
      const auto callback_b = get_callback_or(b->_collisionmapping, a->kind(), std::nullopt);

      if (callback_a) (*callback_a)(a, b);
      if (callback_b) (*callback_b)(b, a);

      SDL_Event event{};
      event.type = static_cast<uint32_t>(type::collision);
      auto ptr = std::make_unique<collision>(a->id(), b->id());
      event.user.data1 = ptr.release();
      SDL_PushEvent(&event);
    }
  }
}

void objectmanager::draw() {
  for (const auto &o : _objects) {
    o->draw();
  }
}

void objectmanager::on_mouse_press(const mouse::button &event) {
  if (event.button != mouse::button::which::left) {
    return;
  }

  const geometry::point point{event.x, event.y};

  const auto clicked = std::ranges::any_of(_objects, [&](const auto &object) {
    const auto &props = object->props();

    if (props.action.empty()) {
      return false;
    }

    const auto it = props.animations.find(props.action);
    if (it == props.animations.end()) {
      return false;
    }

    const auto &animation = it->second;
    if (!animation.hitbox) {
      return false;
    }

    const auto hitbox = geometry::rectangle{
      object->position() + animation.hitbox->position() * props.scale,
      animation.hitbox->size() * props.scale
    };

    if (!hitbox.contains(point)) {
      return false;
    }

    object->on_touch(event.x, event.y);
    return true;
  });

  if (!clicked) {
    _scenemanager->on_touch(event.x, event.y);
  }
}

void objectmanager::on_mouse_motion(const input::event::mouse::motion &event) {
  for (const auto &o : _objects) {
    o->on_motion(event.x, event.y);
  }
}

void objectmanager::on_mail(const input::event::mail &event) {
  if (const auto object = find(event.to); object) {
    object->on_email(event.body);
  }
}
