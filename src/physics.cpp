#include "physics.hpp"

#include "constant.hpp"
#include "geometry.hpp"

using namespace physics;

entt::entity physics::entity_from(b2ShapeId shape) noexcept {
  const auto data = b2Shape_GetUserData(shape);
  return static_cast<entt::entity>(reinterpret_cast<std::uintptr_t>(data));
}

bool physics::valid_pair(b2ShapeId a, b2ShapeId b) noexcept {
  return b2Shape_IsValid(a) & b2Shape_IsValid(b);
}

world::world(const unmarshal::json& node) noexcept {
  auto gx = .0f, gy = .0f;
  if (auto p = node["physics"]) {
    if (auto g = p["gravity"]) {
      gx = g["x"].get(.0f);
      gy = g["y"].get(.0f);
    }
  }

  auto def = b2DefaultWorldDef();
  def.gravity = {gx, gy};
  _id = b2CreateWorld(&def);
}

world::~world() noexcept {
  if (b2World_IsValid(_id)) {
    b2DestroyWorld(_id);
  }
}

world::world(world&& other) noexcept
    : _id(other._id), _accumulator(other._accumulator) {
  other._id = b2WorldId{};
  other._accumulator = 0.0f;
}

world& world::operator=(world&& other) noexcept {
  if (this != &other) {
    if (b2World_IsValid(_id)) {
      b2DestroyWorld(_id);
    }

    _id = other._id;
    _accumulator = other._accumulator;
    other._id = b2WorldId{};
    other._accumulator = 0.0f;
  }

  return *this;
}

void world::step(float delta) noexcept {
  _accumulator += delta;

  while (_accumulator >= FIXED_TIMESTEP) {
    b2World_Step(_id, FIXED_TIMESTEP, WORLD_SUBSTEPS);
    _accumulator -= FIXED_TIMESTEP;
  }
}

b2WorldId world::id() const noexcept {
  return _id;
}

b2SensorEvents world::sensor_events() const noexcept {
  return b2World_GetSensorEvents(_id);
}

std::vector<entt::entity> world::raycast(const vec2& origin, float angle, float distance, category mask) const noexcept {
  const auto radians = angle * (std::numbers::pi_v<float> / 180.0f);
  const auto direction = b2Vec2{std::cos(radians) * distance, std::sin(radians) * distance};
  const auto filter = make_query_filter(category::all, mask);

  struct hit {
    entt::entity entity;
    float fraction;
  };

  boost::container::small_vector<hit, 16> hits;

  b2World_CastRay(
    _id,
    to_b2(origin),
    direction,
    filter,
    [](b2ShapeId shape, b2Vec2, b2Vec2, float fraction, void* userdata) -> float {
      auto* container = static_cast<boost::container::small_vector<hit, 16>*>(userdata);
      container->emplace_back(entity_from(shape), fraction);
      return 1.0f;
    },
    &hits
  );

  std::ranges::sort(hits, {}, &hit::fraction);
  std::vector<entt::entity> result(hits.size());
  std::ranges::transform(hits, result.begin(), &hit::entity);

  return result;
}

body::~body() noexcept {
  destroy();
}

body::body(body&& other) noexcept
    : _body(other._body), _shape(other._shape), _cache(other._cache) {
  other._body = b2BodyId{};
  other._shape = b2ShapeId{};
  other._cache = {};
}

body& body::operator=(body&& other) noexcept {
  if (this != &other) {
    destroy();

    _body = other._body;
    _shape = other._shape;
    _cache = other._cache;
    other._body = b2BodyId{};
    other._shape = b2ShapeId{};
    other._cache = {};
  }

  return *this;
}

body body::create(world& w, const bodydef& d) noexcept {
  body result;
  auto def = b2DefaultBodyDef();
  def.type = static_cast<b2BodyType>(d.type);
  def.position = to_b2(d.position);
  def.userData = reinterpret_cast<void*>(static_cast<std::uintptr_t>(d.entity));
  result._body = b2CreateBody(w.id(), &def);

  if (d.box) {
    result.attach(d.box->x, d.box->y, d.sensor);
  }

  return result;
}

void body::attach(float hx, float hy, bool sensor) noexcept {
  detach();

  const auto poly = b2MakeBox(hx, hy);
  auto def = b2DefaultShapeDef();
  def.isSensor = sensor;
  def.enableSensorEvents = sensor;
  def.userData = b2Body_GetUserData(_body);
  _shape = b2CreatePolygonShape(_body, &def, &poly);
  if (sensor) _cache = {hx, hy};
}

void body::attach_sensor_if_changed(float hx, float hy) noexcept {
  if (has_shape() && _cache.hx == hx && _cache.hy == hy) [[likely]] return;

  attach(hx, hy, true);
}

void body::detach() noexcept {
  if (b2Shape_IsValid(_shape)) {
    b2DestroyShape(_shape, false);
    _shape = b2ShapeId{};
  }
}

void body::destroy() noexcept {
  detach();

  if (b2Body_IsValid(_body)) {
    b2DestroyBody(_body);
    _body = b2BodyId{};
  }
}

void body::set_transform(const vec2& position, float angle) noexcept {
  b2Body_SetTransform(_body, to_b2(position), b2MakeRot(angle));
}

bool body::has_shape() const noexcept {
  return b2Shape_IsValid(_shape);
}

b2BodyId body::id() const noexcept {
  return _body;
}
