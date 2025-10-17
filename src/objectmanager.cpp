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
  for (auto&& item : j["animations"].items()) {
    const std::string& key = item.key();
    const auto& a = item.value();
  
    std::optional<framework::bounds> bounds;
    if (a.contains("bounds")) bounds.emplace(a.at("bounds").get<framework::bounds>());
  
    std::shared_ptr<audio::soundfx> effect;
    if (a.contains("effect")) {
      const std::string e = a.at("effect").get<std::string>();
      effect = _resourcemanager->soundmanager()->get(
        std::format("blobs/{}{}.ogg",
          (scope ? std::format("{}/", scope->get()) : std::string()), e));
    }
  
    std::optional<std::string> next;
    if (a.contains("next")) next.emplace(a.at("next").get<std::string>());
  
    const bool oneshot = next.has_value() || a.value("oneshot", false);
  
    auto keyframes = a.value("frames", std::vector<framework::keyframe>{});
    if (a.contains("bounds") && (!a.contains("frames") || a["frames"].empty())) {
      framework::keyframe k;
      k.duration = -1;
      k.frame = a.at("bounds").get<framework::bounds>().rectangle;
      keyframes.push_back(k);
    }
  
    animations.try_emplace(
      key,
      animation{
        oneshot,
        std::move(next),
        std::move(bounds),
        std::move(effect),
        std::move(keyframes)
      }
    );
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
  std::vector<std::weak_ptr<object>> hits;
  hits.reserve(16);
  _world->query(x, y, std::back_inserter(hits));

  if (hits.empty()) {
    _scenemanager->on_touch(x, y);
    return;
  }

  for (const auto& weak : hits) {
    if (auto o = weak.lock()) {
      o->on_touch(x, y);
    }
  }
}

constexpr auto owner_eq = [](const std::weak_ptr<object>& a, const std::shared_ptr<object>& b) {
  return !a.owner_before(b) && !b.owner_before(a);
};

void objectmanager::on_mouse_motion(const input::event::mouse::motion& event) {
  const auto& x = event.x;
  const auto& y = event.y;

  std::vector<std::weak_ptr<object>> hits;
  hits.reserve(16);
  _world->query(x, y, std::back_inserter(hits));

  std::unordered_set<uint64_t> current;
  current.reserve(hits.size());

  for (const auto& w : hits) {
    const auto p = w.lock();
    if (!p) continue;
    current.insert(p->id());
  }

  for (const auto id : _hovering) {
    if (current.find(id) != current.end()) continue;
    const auto o = find(id);
    if (!o) continue;
    o->on_unhover();
  }

  for (const auto id : current) {
    if (_hovering.find(id) != _hovering.end()) continue;
    const auto o = find(id);
    if (!o) continue;
    o->on_hover();
  }

  _hovering = std::move(current);
}

void objectmanager::on_mail(const input::event::mail& event) {
  if (const auto& o = find(event.to); o) {
    o->on_email(event.body);
  }
}
