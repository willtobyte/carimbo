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
  _hovering.reserve(256);
  _objects.get<by_id>().rehash(128);
}

std::shared_ptr<object> objectmanager::create(const std::string& kind, std::optional<std::reference_wrapper<const std::string>> scope, bool manage) {
  static const std::string empty;
  const auto& n = scope.value_or(std::cref(empty)).get();
  const auto& qualifier = n.empty() ? kind : std::format("{}/{}", n, kind);

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
  o->_scale = scale;
  o->_kind = kind;
  o->_scope = n;
  o->_animations = std::move(animations);
  o->_spritesheet = std::move(spritesheet);

  o->_world = _world;
  o->_objectmanager = shared_from_this();

  std::println("[objectmanager] created {} {}", o->kind(), o->id());

  if (manage) {
    _objects.emplace(o);
  }

  return o;
}

std::shared_ptr<object> objectmanager::clone(std::shared_ptr<object> matrix) {
  if (!matrix) [[unlikely]] {
    return nullptr;
  }

  const auto o = std::make_shared<object>();
  o->_angle = matrix->_angle;
  o->_kind = matrix->_kind;
  o->_scope = matrix->_scope;
  o->_action = matrix->_action;
  o->_spritesheet = matrix->_spritesheet;
  o->_animations = matrix->_animations;
  o->_position = matrix->_position;
  o->_scale = matrix->_scale;
  o->_reflection = matrix->_reflection;
  o->_alpha = matrix->_alpha;

  o->_world = _world;
  o->_objectmanager = shared_from_this();

  _objects.emplace(o);

  std::println("[objectmanager] clone {} to {}", matrix->kind(), o->id());

  return o;
}

void objectmanager::manage(std::shared_ptr<object> object) noexcept {
  if (!object) [[unlikely]] {
    return;
  }

  _objects.emplace(object);
}

bool objectmanager::remove(std::shared_ptr<object> object) noexcept {
  if (!object) [[unlikely]] {
    return false;
  }

  const auto id = object->id();

  _hovering.erase(id);

  if (b2Body_IsValid(object->body)) {
    b2DestroyBody(object->body);
    object->body = b2BodyId{};
  }

  return _objects.get<by_id>().erase(id) > 0;
}

std::shared_ptr<object> objectmanager::find(uint64_t id) const noexcept {
  const auto& byid = _objects.get<by_id>();
  auto it = byid.find(id);
  if (it == byid.end()) [[unlikely]] return nullptr;
  return it->object;
}

void objectmanager::update(float delta) noexcept {
  const auto now = SDL_GetTicks();
  const auto& byseq = _objects.get<by_seq>();
  for (const auto& e : byseq) {
    e.object->update(delta, now);
  }
}

void objectmanager::draw() const noexcept {
  const auto& byseq = _objects.get<by_seq>();
  for (const auto& e : byseq) {
    e.object->draw();
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
  if (event.button != mouse::button::which::left) return;

  const auto x = event.x;
  const auto y = event.y;

  static std::vector<uint64_t> hits;
  hits.clear();
  hits.reserve(32);
  _world->query(x, y, std::back_inserter(hits));
  if (hits.empty()) [[likely]] {
    _scenemanager->on_touch(x, y);
    return;
  }

  for (auto id : hits) {
    if (const auto& o = find(id)) [[likely]] {
      o->on_touch(x, y);
    }
  }
}

void objectmanager::on_mouse_motion(const input::event::mouse::motion& event) {
  const auto x = event.x;
  const auto y = event.y;

  static std::unordered_set<uint64_t> hits;
  hits.clear();
  hits.reserve(32);
  _world->query(x, y, std::inserter(hits, hits.end()));

  for (const auto id : _hovering) {
    if (hits.contains(id)) continue;
    if (const auto& o = find(id)) [[likely]] {
      o->on_unhover();
    }
  }

  for (const auto id : hits) {
    if (_hovering.contains(id)) continue;
    if (const auto& o = find(id)) [[likely]] {
      o->on_hover();
    }
  }

  _hovering.swap(hits);
}

void objectmanager::on_mail(const input::event::mail& event) {
  if (const auto& o = find(event.to); o) {
    o->on_email(event.body);
  }
}
