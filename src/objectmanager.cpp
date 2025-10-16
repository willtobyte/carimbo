#include "objectmanager.hpp"
#include "object.hpp"

using namespace framework;

using namespace input::event;

template <typename T>
std::optional<std::function<T>> operator||(const std::optional<std::function<T>>& lhs, const std::optional<std::function<T>>& rhs) {
  return lhs ? lhs : rhs;
}

objectmanager::objectmanager() {
  _envelopepool->reserve(64);
  _objects.reserve(256);
  _hovering.reserve(256);
}

std::shared_ptr<object> objectmanager::create(const std::string& kind, std::optional<std::reference_wrapper<const std::string>> scope, bool manage) {
  static const std::string empty;
  const auto& n = scope.value_or(std::cref(empty)).get();
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

  const auto scale = j.value("scale", float{1.f});
  const auto spritesheet = _resourcemanager->pixmappool()->get(std::format("blobs/{}.png", qualifier));
  std::unordered_map<std::string, animation> animations;
  animations.reserve(j["animations"].size());
  for (const auto& item : j["animations"].items()) {
    const auto& key = item.key();
    const auto& a = item.value();
    const auto bounds = a.contains("bounds") ? std::make_optional(a.at("bounds").template get<framework::bounds>()) : std::nullopt;
    const auto effect = a.contains("effect") ? _resourcemanager->soundmanager()->get(std::format("blobs/{}{}.ogg", scope ? std::format("{}/", scope->get()) : std::string(), a.at("effect").template get_ref<const std::string&>())) : nullptr;
    const auto next = a.contains("next") ? std::make_optional(a.at("next").template get_ref<const std::string&>()) : std::nullopt;
    const bool oneshot = next.has_value() || a.value("oneshot", false);
    const auto keyframes = a.value("frames", std::vector<framework::keyframe>{});

    animations.emplace(key, animation{oneshot, next, bounds, effect, std::move(keyframes)});
  }

  auto o = std::make_shared<object>();
  o->_id = _counter++;
  o->_scale = scale;
  o->_kind = kind;
  o->_scope = n;
  o->_animations = std::move(animations);
  o->_spritesheet = std::move(spritesheet);

  std::println("[objectmanager] created {} {}", qualifier, o->id());
  if (manage) {
    _world->add(o);
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

  std::println("[objectmanager] clone {} from {}", o->id(), matrix->id());

  _world->add(o);
  return o;
}

void objectmanager::manage(std::shared_ptr<object> object) noexcept {
  if (!object) [[unlikely]] {
    return;
  }

  _world->add(object);
  _objects.emplace_back(object);
}

void objectmanager::remove(std::shared_ptr<object> object) noexcept {
  if (!object) [[unlikely]] {
    return;
  }

  _world->remove(object);

  const auto it = std::find(_objects.begin(), _objects.end(), object);
  if (it == _objects.end()) [[unlikely]] {
    return;
  }

  if (it != _objects.end() - 1) [[likely]] {
    std::iter_swap(it, _objects.end() - 1);
  }

  _objects.pop_back();
}

std::shared_ptr<object> objectmanager::find(uint64_t id) const noexcept {
  for (const auto& o : _objects) {
    if (o->id() == id) {
      return o;
    }
  }

  return nullptr;
}

void objectmanager::update(float delta) noexcept {
  for (const auto& o : _objects) {
    o->update(delta);
  }
}

void objectmanager::draw() const noexcept {
  for (const auto& o : _objects) {
    o->draw();
  }
}

void objectmanager::set_resourcemanager(std::shared_ptr<resourcemanager> resourcemanager) noexcept {
  _resourcemanager = std::move(resourcemanager);
}

void objectmanager::set_scenemanager(std::shared_ptr<scenemanager> scenemanager) noexcept {
  _scenemanager = std::move(scenemanager);
}

void objectmanager::set_world(std::shared_ptr<world> world) noexcept {
  _world = std::move(world);
}

void objectmanager::on_mouse_release(const mouse::button& event) {
  if (event.button != mouse::button::which::left) {
    return;
  }

  const auto& x = event.x;
  const auto& y = event.y;
  std::vector<std::weak_ptr<object>> objects;
  objects.reserve(16);
  _world->query(x, y, std::back_inserter(objects));

  if (objects.empty()) {
    _scenemanager->on_touch(x, y);
    return;
  }

  for (const auto& weak : objects) {
    if (auto o = weak.lock()) {
      o->on_touch(x, y);
    }
  }
}

void objectmanager::on_mouse_motion(const input::event::mouse::motion& event) {
  const auto& x = event.x;
  const auto& y = event.y;

  std::vector<std::weak_ptr<object>> objects;
  objects.reserve(16);
  _world->query(x, y, std::back_inserter(objects));

  static auto owner_eq = [](const std::weak_ptr<object>& a, const std::shared_ptr<object>& b) {
    return !a.owner_before(b) && !b.owner_before(a);
  };

  for (const auto& weak : _hovering) {
    const auto prev = weak.lock();
    if (!prev) continue;

    bool over = false;
    for (const auto& weak : objects) {
      if (const auto current = weak.lock(); current && owner_eq(weak, prev)) { over = true; break; }
    }

    if (!over) prev->on_unhover();
  }

  _hovering.clear();

  for (const auto& weak : objects) {
    const auto object = weak.lock();
    if (!object) continue;
    object->on_hover();
    _hovering.emplace_back(weak);
  }
}

void objectmanager::on_mail(const input::event::mail& event) {
  if (const auto& o = find(event.to); o) {
    o->on_email(event.body);
  }
}
