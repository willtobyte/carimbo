#include "physics.hpp"

#include "constant.hpp"
#include "geometry.hpp"

using namespace physics;

entt::entity physics::entity_from(b2ShapeId shape) noexcept {
  const auto b = b2Shape_GetBody(shape);
  const auto data = b2Body_GetUserData(b);
  return static_cast<entt::entity>(reinterpret_cast<std::uintptr_t>(data));
}

bool physics::valid_pair(b2ShapeId a, b2ShapeId b) noexcept {
  return b2Shape_IsValid(a) & b2Shape_IsValid(b);
}

quad physics::shape_aabb(b2ShapeId shape) noexcept {
  if (!b2Shape_IsValid(shape)) return {};

  const auto aabb = b2Shape_GetAABB(shape);

  return {
    aabb.lowerBound.x, aabb.lowerBound.y,
    aabb.upperBound.x - aabb.lowerBound.x,
    aabb.upperBound.y - aabb.lowerBound.y
  };
}

world::world(float gravity_x, float gravity_y) noexcept {
  auto def = b2DefaultWorldDef();
  def.gravity = {gravity_x, gravity_y};
  _id = b2CreateWorld(&def);
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

bool world::valid() const noexcept {
  return b2World_IsValid(_id);
}

world::operator bool() const noexcept {
  return valid();
}

b2SensorEvents world::sensor_events() const noexcept {
  return b2World_GetSensorEvents(_id);
}

std::optional<raycast_hit> world::raycast_closest(const vec2& origin, const vec2& target, category mask) const noexcept {
  const auto filter = make_query_filter(category::all, mask);
  const auto o = to_b2(origin);
  const auto t = to_b2(target);
  const auto result = b2World_CastRayClosest(_id, o, {t.x - o.x, t.y - o.y}, filter);

  if (!result.hit) [[likely]] {
    return std::nullopt;
  }

  return raycast_hit{result.point, result.normal, result.fraction, result.shapeId, entity_from(result.shapeId)};
}

body::~body() noexcept {
  destroy();
}

body::body(body&& other) noexcept
    : _body(other._body), _shape(other._shape) {
  other._body = b2BodyId{};
  other._shape = b2ShapeId{};
}

body& body::operator=(body&& other) noexcept {
  if (this != &other) {
    destroy();

    _body = other._body;
    _shape = other._shape;
    other._body = b2BodyId{};
    other._shape = b2ShapeId{};
  }

  return *this;
}

body body::create(world& w, bodytype type, const vec2& position, entt::entity entity) noexcept {
  body result;
  auto def = b2DefaultBodyDef();
  def.type = static_cast<b2BodyType>(type);
  def.position = to_b2(position);
  def.userData = reinterpret_cast<void*>(static_cast<std::uintptr_t>(entity));
  result._body = b2CreateBody(w.id(), &def);

  return result;
}

body body::create_static(world& w, const vec2& position) noexcept {
  body result;
  auto def = b2DefaultBodyDef();
  def.type = b2_staticBody;
  def.position = to_b2(position);
  result._body = b2CreateBody(w.id(), &def);

  return result;
}

body body::create_static(world& w, const vec2& position, const vec2& half_extents) noexcept {
  auto result = create_static(w, position);
  result.attach_box(half_extents.x, half_extents.y);

  return result;
}

void body::attach_sensor(float hx, float hy) noexcept {
  detach_shape();

  const auto poly = b2MakeBox(hx, hy);
  auto def = b2DefaultShapeDef();
  def.isSensor = true;
  def.enableSensorEvents = true;
  _shape = b2CreatePolygonShape(_body, &def, &poly);
}

void body::attach_circle_sensor(float radius, const vec2& offset) noexcept {
  detach_shape();

  b2Circle circle;
  circle.center = to_b2(offset);
  circle.radius = radius;
  auto def = b2DefaultShapeDef();
  def.isSensor = true;
  def.enableSensorEvents = true;
  _shape = b2CreateCircleShape(_body, &def, &circle);
}

void body::attach_capsule_sensor(const vec2& p1, const vec2& p2, float radius) noexcept {
  detach_shape();

  b2Capsule capsule;
  capsule.center1 = to_b2(p1);
  capsule.center2 = to_b2(p2);
  capsule.radius = radius;
  auto def = b2DefaultShapeDef();
  def.isSensor = true;
  def.enableSensorEvents = true;
  _shape = b2CreateCapsuleShape(_body, &def, &capsule);
}

void body::attach_polygon_sensor(std::span<const b2Vec2> vertices) noexcept {
  detach_shape();

  const auto hull = b2ComputeHull(vertices.data(), static_cast<int32_t>(vertices.size()));
  const auto poly = b2MakePolygon(&hull, 0.0f);
  auto def = b2DefaultShapeDef();
  def.isSensor = true;
  def.enableSensorEvents = true;
  _shape = b2CreatePolygonShape(_body, &def, &poly);
}

void body::attach_box(float hx, float hy, float density, float friction) noexcept {
  detach_shape();

  const auto poly = b2MakeBox(hx, hy);
  auto def = b2DefaultShapeDef();
  def.density = density;
  def.material.friction = friction;
  _shape = b2CreatePolygonShape(_body, &def, &poly);
}

void body::attach_circle(float radius, float density, float friction) noexcept {
  detach_shape();

  b2Circle circle;
  circle.center = {0.0f, 0.0f};
  circle.radius = radius;
  auto def = b2DefaultShapeDef();
  def.density = density;
  def.material.friction = friction;
  _shape = b2CreateCircleShape(_body, &def, &circle);
}

void body::attach_capsule(const vec2& p1, const vec2& p2, float radius, float density, float friction) noexcept {
  detach_shape();

  b2Capsule capsule;
  capsule.center1 = to_b2(p1);
  capsule.center2 = to_b2(p2);
  capsule.radius = radius;
  auto def = b2DefaultShapeDef();
  def.density = density;
  def.material.friction = friction;
  _shape = b2CreateCapsuleShape(_body, &def, &capsule);
}

void body::attach_polygon(std::span<const b2Vec2> vertices, float density, float friction) noexcept {
  detach_shape();

  const auto hull = b2ComputeHull(vertices.data(), static_cast<int32_t>(vertices.size()));
  const auto poly = b2MakePolygon(&hull, 0.0f);
  auto def = b2DefaultShapeDef();
  def.density = density;
  def.material.friction = friction;
  _shape = b2CreatePolygonShape(_body, &def, &poly);
}

void body::detach_shape() noexcept {
  if (b2Shape_IsValid(_shape)) {
    b2DestroyShape(_shape, false);
    _shape = b2ShapeId{};
  }
}

void body::destroy() noexcept {
  detach_shape();

  if (b2Body_IsValid(_body)) {
    b2DestroyBody(_body);
    _body = b2BodyId{};
  }
}

void body::set_transform(const vec2& position, float angle) noexcept {
  b2Body_SetTransform(_body, to_b2(position), b2MakeRot(angle));
}

void body::set_position(const vec2& position) noexcept {
  b2Body_SetTransform(_body, to_b2(position), b2Body_GetRotation(_body));
}

void body::set_linear_velocity(const vec2& velocity) noexcept {
  b2Body_SetLinearVelocity(_body, to_b2(velocity));
}

void body::set_angular_velocity(float omega) noexcept {
  b2Body_SetAngularVelocity(_body, omega);
}

void body::set_gravity_scale(float scale) noexcept {
  b2Body_SetGravityScale(_body, scale);
}

void body::set_fixed_rotation(bool fixed) noexcept {
  b2Body_SetFixedRotation(_body, fixed);
}

void body::set_bullet(bool bullet) noexcept {
  b2Body_SetBullet(_body, bullet);
}

void body::set_enabled(bool enabled) noexcept {
  if (enabled) {
    b2Body_Enable(_body);
  } else {
    b2Body_Disable(_body);
  }
}

void body::set_filter(category cat, category mask) noexcept {
  if (!b2Shape_IsValid(_shape)) return;

  b2Shape_SetFilter(_shape, make_filter(cat, mask));
}

void body::apply_force(const vec2& force, const vec2& point) noexcept {
  b2Body_ApplyForce(_body, to_b2(force), to_b2(point), true);
}

void body::apply_force_center(const vec2& force) noexcept {
  b2Body_ApplyForceToCenter(_body, to_b2(force), true);
}

void body::apply_impulse(const vec2& impulse, const vec2& point) noexcept {
  b2Body_ApplyLinearImpulse(_body, to_b2(impulse), to_b2(point), true);
}

void body::apply_impulse_center(const vec2& impulse) noexcept {
  b2Body_ApplyLinearImpulseToCenter(_body, to_b2(impulse), true);
}

void body::apply_torque(float torque) noexcept {
  b2Body_ApplyTorque(_body, torque, true);
}

vec2 body::position() const noexcept {
  return from_b2(b2Body_GetPosition(_body));
}

float body::angle() const noexcept {
  return b2Rot_GetAngle(b2Body_GetRotation(_body));
}

vec2 body::linear_velocity() const noexcept {
  return from_b2(b2Body_GetLinearVelocity(_body));
}

float body::angular_velocity() const noexcept {
  return b2Body_GetAngularVelocity(_body);
}

bool body::valid() const noexcept {
  return b2Body_IsValid(_body);
}

bool body::has_shape() const noexcept {
  return b2Shape_IsValid(_shape);
}

entt::entity body::entity() const noexcept {
  const auto data = b2Body_GetUserData(_body);
  return static_cast<entt::entity>(reinterpret_cast<std::uintptr_t>(data));
}

quad body::shape_aabb() const noexcept {
  if (!b2Shape_IsValid(_shape)) return {};

  const auto aabb = b2Shape_GetAABB(_shape);

  return {
    aabb.lowerBound.x, aabb.lowerBound.y,
    aabb.upperBound.x - aabb.lowerBound.x,
    aabb.upperBound.y - aabb.lowerBound.y
  };
}

body::operator bool() const noexcept {
  return valid();
}

b2BodyId body::id() const noexcept {
  return _body;
}

b2ShapeId body::shape_id() const noexcept {
  return _shape;
}

joint::~joint() noexcept {
  destroy();
}

joint::joint(joint&& other) noexcept
    : _id(other._id), _ground(other._ground) {
  other._id = b2JointId{};
  other._ground = b2BodyId{};
}

joint& joint::operator=(joint&& other) noexcept {
  if (this != &other) {
    destroy();

    _id = other._id;
    _ground = other._ground;
    other._id = b2JointId{};
    other._ground = b2BodyId{};
  }

  return *this;
}

joint joint::distance(world& w, body& a, body& b, const vec2& anchor_a, const vec2& anchor_b, float length) noexcept {
  joint result;
  auto def = b2DefaultDistanceJointDef();
  def.bodyIdA = a.id();
  def.bodyIdB = b.id();
  def.localAnchorA = to_b2(anchor_a);
  def.localAnchorB = to_b2(anchor_b);

  if (length > 0.0f) {
    def.length = length;
  } else {
    const auto pa = b2Body_GetWorldPoint(a.id(), def.localAnchorA);
    const auto pb = b2Body_GetWorldPoint(b.id(), def.localAnchorB);
    def.length = b2Length(b2Sub(pb, pa));
  }

  result._id = b2CreateDistanceJoint(w.id(), &def);

  return result;
}

joint joint::revolute(world& w, body& a, body& b, const vec2& anchor) noexcept {
  joint result;
  auto def = b2DefaultRevoluteJointDef();
  def.bodyIdA = a.id();
  def.bodyIdB = b.id();
  def.localAnchorA = b2Body_GetLocalPoint(a.id(), to_b2(anchor));
  def.localAnchorB = b2Body_GetLocalPoint(b.id(), to_b2(anchor));
  result._id = b2CreateRevoluteJoint(w.id(), &def);

  return result;
}

joint joint::prismatic(world& w, body& a, body& b, const vec2& anchor, const vec2& axis) noexcept {
  joint result;
  auto def = b2DefaultPrismaticJointDef();
  def.bodyIdA = a.id();
  def.bodyIdB = b.id();
  def.localAnchorA = b2Body_GetLocalPoint(a.id(), to_b2(anchor));
  def.localAnchorB = b2Body_GetLocalPoint(b.id(), to_b2(anchor));
  def.localAxisA = to_b2(axis);
  result._id = b2CreatePrismaticJoint(w.id(), &def);

  return result;
}

joint joint::motor(world& w, body& a, body& b, float max_force, float max_torque) noexcept {
  joint result;
  auto def = b2DefaultMotorJointDef();
  def.bodyIdA = a.id();
  def.bodyIdB = b.id();
  def.maxForce = max_force;
  def.maxTorque = max_torque;
  result._id = b2CreateMotorJoint(w.id(), &def);

  return result;
}

joint joint::weld(world& w, body& a, body& b, const vec2& anchor) noexcept {
  joint result;
  auto def = b2DefaultWeldJointDef();
  def.bodyIdA = a.id();
  def.bodyIdB = b.id();
  def.localAnchorA = b2Body_GetLocalPoint(a.id(), to_b2(anchor));
  def.localAnchorB = b2Body_GetLocalPoint(b.id(), to_b2(anchor));
  result._id = b2CreateWeldJoint(w.id(), &def);

  return result;
}

joint joint::wheel(world& w, body& chassis, body& whl, const vec2& anchor, const vec2& axis) noexcept {
  joint result;
  auto def = b2DefaultWheelJointDef();
  def.bodyIdA = chassis.id();
  def.bodyIdB = whl.id();
  def.localAnchorA = b2Body_GetLocalPoint(chassis.id(), to_b2(anchor));
  def.localAnchorB = b2Body_GetLocalPoint(whl.id(), to_b2(anchor));
  def.localAxisA = to_b2(axis);
  result._id = b2CreateWheelJoint(w.id(), &def);

  return result;
}

joint joint::mouse(world& w, body& b, const vec2& target, float max_force) noexcept {
  joint result;
  auto ground_def = b2DefaultBodyDef();
  result._ground = b2CreateBody(w.id(), &ground_def);
  auto def = b2DefaultMouseJointDef();
  def.bodyIdA = result._ground;
  def.bodyIdB = b.id();
  def.target = to_b2(target);
  def.maxForce = max_force;
  result._id = b2CreateMouseJoint(w.id(), &def);

  return result;
}

void joint::set_target(const vec2& target) noexcept {
  if (!b2Joint_IsValid(_id)) return;

  b2MouseJoint_SetTarget(_id, to_b2(target));
}

void joint::destroy() noexcept {
  if (b2Joint_IsValid(_id)) {
    b2DestroyJoint(_id);
    _id = b2JointId{};
  }

  if (b2Body_IsValid(_ground)) {
    b2DestroyBody(_ground);
    _ground = b2BodyId{};
  }
}

bool joint::valid() const noexcept {
  return b2Joint_IsValid(_id);
}

joint::operator bool() const noexcept {
  return valid();
}
