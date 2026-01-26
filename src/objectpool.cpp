#include "objectpool.hpp"

#include "components.hpp"
#include "io.hpp"
#include "objectproxy.hpp"
#include "pixmap.hpp"

objectpool::objectpool(
    entt::registry& registry,
    physics::world& world,
    std::string_view scenename,
    sol::environment& environment
)
    : _registry(registry),
      _world(world),
      _scenename(scenename),
      _environment(environment) {
}

void objectpool::add(unmarshal::json node, int32_t z) {
  auto& interning = _registry.ctx().get<::interning>();

  const auto name = node["name"].get<std::string_view>();
  const auto kind = node["kind"].get<std::string_view>();
  const auto action = interning.intern(node["action"].get<std::string_view>());
  const auto position = node.get<vec2>();

  const auto entity = _registry.create();

  auto [it, inserted] = _shared.try_emplace(kind);
  if (inserted) {
    const auto json = unmarshal::parse(io::read(std::format("objects/{}/{}.json", _scenename, kind)));

    auto atlas = std::make_shared<::atlas>();
    if (auto timelines = json["timelines"]) {
      timelines.foreach([&atlas, &interning](std::string_view key, unmarshal::json node) {
        timeline tl{};
        tl.oneshot = node["oneshot"].get(false);

        if (auto nextnode = node["next"]) {
          tl.next = interning.intern(nextnode.get<std::string_view>());
        }

        if (auto value = node["hitbox"]) {
          if (auto aabb = value["aabb"]) {
            tl.hitbox = aabb.get<quad>();
          }
        }

        if (auto frames = node["frames"]) {
          tl.frames.reserve(frames.size());
          frames.foreach([&tl](unmarshal::json f) {
            tl.frames.emplace_back(std::move(f));
          });
        }

        atlas->timelines.try_emplace(interning.intern(key), std::move(tl));
      });
    }

    it->second = shared{
      .atlas = std::move(atlas),
      .pixmap = std::make_shared<::pixmap>(std::format("blobs/{}/{}.png", _scenename, kind)),
      .scale = json["scale"].get(1.0f)
    };
  }

  _registry.emplace<const ::atlas*>(entity, it->second.atlas.get());
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
  _registry.emplace<sprite>(entity, sprite{.pixmap = it->second.pixmap.get()});
  _registry.emplace<playback>(entity, playback{
    .current_frame = 0,
    .tick = SDL_GetTicks(),
    .action = action,
    .timeline = nullptr
  });
  _registry.emplace<dirtable>(entity);
  _registry.emplace<drawable>(entity);
  _registry.emplace<orientation>(entity);
  _registry.emplace<velocity>(entity);
  _registry.emplace<physics::body>(entity, physics::body::create(_world, {.type = physics::bodytype::kinematic, .position = position, .entity = entity}));
  _registry.emplace<renderable>(entity, renderable{.z = z});

  const auto proxy = std::make_shared<objectproxy>(entity, _registry);
  _registry.emplace<std::shared_ptr<objectproxy>>(entity, proxy);

  auto& scripting = _registry.ctx().get<::scripting>();
  scripting.wire(entity, _environment, proxy, std::format("objects/{}/{}.lua", _scenename, kind));
}

void objectpool::populate(sol::table& pool) const {
  auto& interning = _registry.ctx().get<::interning>();
  for (auto&& [entity, meta, proxy] : _registry.view<metadata, std::shared_ptr<objectproxy>>().each()) {
    const auto name = interning.lookup(meta.name);
    assert(!pool[name].valid() && "duplicate key in pool");
    pool[name] = proxy;
  }
}

void objectpool::sort() {
  _registry.sort<renderable>([](const renderable& lhs, const renderable& rhs) {
    return lhs.z < rhs.z;
  });
}

void objectpool::draw(entt::entity entity) const noexcept {
  const auto& [pb, tr, tn, sp, fl, dr] = _registry.get<playback, transform, tint, sprite, orientation, drawable>(entity);

  if (!pb.timeline || pb.timeline->frames.empty()) [[unlikely]] return;

  const auto& frame = pb.timeline->frames[pb.current_frame];
  const auto& q = frame.quad;
  sp.pixmap->draw(q.x, q.y, q.w, q.h, dr.x, dr.y, dr.w, dr.h, tr.angle, tn.a, fl.flip);
}
