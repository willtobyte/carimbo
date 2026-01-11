#include "objectmanager.hpp"

#include "components.hpp"
#include "io.hpp"
#include "objectproxy.hpp"
#include "pixmap.hpp"
#include "renderer.hpp"

objectmanager::objectmanager(
    entt::registry& registry,
    std::shared_ptr<renderer> renderer,
    std::string_view scenename,
    sol::environment& environment
)
    : _registry(registry),
      _renderer(std::move(renderer)),
      _scenename(scenename),
      _environment(environment) {
  _proxies.reserve(32);
}

void objectmanager::add(unmarshal::json node, int32_t z) {
  auto& interning = _registry.ctx().get<::interning>();

  const auto name = node["name"].get<std::string_view>();
  const auto kind = node["kind"].get<std::string_view>();
  const auto action = interning.intern(node["action"].get<std::string_view>());
  const auto position = node.get<vec2>();

  const auto entity = _registry.create();

  auto [it, inserted] = _props.try_emplace(kind);
  if (inserted) {
    const auto json = unmarshal::parse(io::read(std::format("objects/{}/{}.json", _scenename, kind)));

    auto at = std::make_shared<::atlas>();
    if (auto timelines = json["timelines"]) {
      timelines.foreach([&at, &interning](std::string_view key, unmarshal::json node) {
        timeline tl{};
        tl.oneshot = node["oneshot"].get(false);

        if (auto nextnode = node["next"]) {
          tl.next = interning.intern(nextnode.get<std::string_view>());
        }

        if (auto value = node["hitbox"]) {
          if (auto aabb = value["aabb"]) {
            const auto q = aabb.get<quad>();
            tl.hitbox = b2AABB{
              .lowerBound = b2Vec2(q.x - epsilon, q.y - epsilon),
              .upperBound = b2Vec2(q.x + q.w + epsilon, q.y + q.h + epsilon)
            };
          }
        }

        if (auto frames = node["frames"]) {
          tl.frames.reserve(frames.size());
          frames.foreach([&tl](unmarshal::json f) {
            tl.frames.emplace_back(std::move(f));
          });
        }

        at->timelines.try_emplace(interning.intern(key), std::move(tl));
      });
    }

    it->second = props{
      .atlas = std::move(at),
      .pixmap = std::make_shared<::pixmap>(_renderer, std::format("blobs/{}/{}.png", _scenename, kind)),
      .scale = json["scale"].get(1.0f)
    };
  }

  _registry.emplace<std::shared_ptr<const ::atlas>>(entity, it->second.atlas);
  _registry.emplace<transform>(entity, transform{
    .position = position,
    .angle = .0,
    .scale = it->second.scale
  });
  _registry.emplace<metadata>(entity, metadata{
    .kind = interning.intern(kind),
    .name = interning.intern(name)
  });
  _registry.emplace<tint>(entity);
  _registry.emplace<sprite>(entity, sprite{.pixmap = it->second.pixmap});
  _registry.emplace<playback>(entity, playback{
    .dirty = true,
    .redraw = false,
    .current_frame = 0,
    .tick = SDL_GetTicks(),
    .action = action,
    .timeline = nullptr
  });
  _registry.emplace<orientation>(entity);
  _registry.emplace<velocity>(entity);
  _registry.emplace<rigidbody>(entity);
  _registry.emplace<renderable>(entity, renderable{.z = z});

  const auto proxy = std::make_shared<objectproxy>(entity, _registry);
  _proxies.try_emplace(name, proxy);

  auto& scripting = _registry.ctx().get<::scripting>();
  scripting.wire(entity, _environment, proxy, std::format("objects/{}/{}.lua", _scenename, kind));
}

void objectmanager::populate(sol::table& pool) const {
  for (const auto& [name, proxy] : _proxies) {
    assert(!pool[name].valid() && "duplicate key in pool");
    pool[name] = proxy;
  }
}

void objectmanager::sort() {
  _registry.sort<renderable>([](const renderable& lhs, const renderable& rhs) {
    return lhs.z < rhs.z;
  });
}
