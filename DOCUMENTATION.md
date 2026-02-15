# Carimbo Engine — Complete Lua API & Project Structure Reference

> **Audience**: LLMs, AI coding agents, and automated code generators.
> This document is the single source of truth for the Carimbo game engine's Lua scripting API and project conventions. Every type, function, property, callback, enum, JSON schema, and file convention is documented below. When generating code for a Carimbo game, follow these specifications exactly.

---

## Table of Contents

1. [Project Structure](#1-project-structure)
2. [Engine Bootstrap — EngineFactory](#2-engine-bootstrap--enginefactory)
3. [Global Functions](#3-global-functions)
4. [Global Instances](#4-global-instances)
5. [Math Overrides](#5-math-overrides)
6. [Types (Usertypes)](#6-types-usertypes)
7. [Enums](#7-enums)
8. [SceneManager](#8-scenemanager)
9. [Scene Module Callbacks](#9-scene-module-callbacks)
10. [Entity (Object Proxy)](#10-entity-object-proxy)
11. [Object Script Module Callbacks](#11-object-script-module-callbacks)
12. [Observable](#12-observable)
13. [SoundFX](#13-soundfx)
14. [Overlay](#14-overlay)
15. [Label](#15-label)
16. [Cursor](#16-cursor)
17. [Cassette (Persistent Save)](#17-cassette-persistent-save)
18. [ParticleProps](#18-particleprops)
19. [ParticlePool](#19-particlepool)
20. [Achievement](#20-achievement)
21. [User & Buddy](#21-user--buddy)
22. [OperatingSystem](#22-operatingsystem)
23. [Desktop](#23-desktop)
24. [Keyboard](#24-keyboard)
25. [Gamepad](#25-gamepad)
26. [World (Per-Scene Physics)](#26-world-per-scene-physics)
27. [JSON Schemas](#27-json-schemas)
28. [Complete Examples](#28-complete-examples)

---

## 1. Project Structure

A Carimbo game (called a "cartridge.rom", a zip archive) is a directory with the following layout. All paths are relative to the cartridge root.

```
<cartridge>/
  scripts/
    main.lua                        # REQUIRED — engine entry point
    <module>.lua                    # shared scripts (loaded via require("<module>"))
    <subdir>/<module>.lua           # nested modules (loaded via require("<subdir>/<module>"))

  scenes/
    <scenename>.json                # REQUIRED per scene — declarative scene definition
    <scenename>.lua                 # REQUIRED per scene — scene behavior (Lua module)

  objects/
    <scenename>/
      <kind>.json                   # object sprite/animation definition
      <kind>.lua                    # object behavior (Lua module, optional for static objects)

  blobs/
    <scenename>/
      background.png                # scene background image (when layer type is "background")
      <kind>.png                    # object spritesheet
      <sound>.opus                  # sound effect (Opus format)
    overlay/
      <fontfamily>.png              # font spritesheet
      <cursorname>.png              # cursor spritesheet
    particles/
      <particlekind>.png            # particle texture
    tilemaps/
      <tilemapname>.png             # tilemap atlas spritesheet

  fonts/
    <fontfamily>.json               # font descriptor

  cursors/
    <cursorname>.json               # cursor animation descriptor

  particles/
    <particlekind>.json             # particle emitter configuration

  tilemaps/
    <tilemapname>.json              # tilemap grid data

  locales/
    <lang>.json                     # localization strings (e.g., "en.json", "pt.json")
```

### Key Rules

- **`scripts/main.lua`** is the only mandatory Lua file. The engine loads and executes it first.
- `main.lua` MUST define a global `engine` variable (via `EngineFactory`) and a global `setup()` function.
- The engine calls `setup()` once after `main.lua` executes. This is where you register and set the initial scene.
- Shared scripts are loaded via standard `require("module")` or `require("subdir/module")`. The engine injects a custom searcher that loads from `scripts/`.
- Each scene requires both a `.json` and `.lua` file in `scenes/`.
- Each object requires a `.json` file in `objects/<scenename>/`. The `.lua` file is optional; objects without a Lua file are purely visual/static.
- Asset files (PNG, Opus) go in `blobs/` following the naming conventions above.

---

## 2. Engine Bootstrap — EngineFactory

The `EngineFactory` creates and configures the engine. It uses a builder/fluent pattern.

### Constructor

```lua
local factory = EngineFactory.new()
```

### Builder Methods

All methods return `self` for chaining.

| Method | Parameter | Default | Description |
|--------|-----------|---------|-------------|
| `with_title(title)` | `string` | `"Untitled"` | Window title |
| `with_width(width)` | `integer` | `800` | Window width in pixels |
| `with_height(height)` | `integer` | `600` | Window height in pixels |
| `with_scale(scale)` | `float` | `1.0` | Render scale factor. The logical resolution is `width/scale` x `height/scale`. |
| `with_gravity(gravity)` | `float` | `9.8` | Default gravity Y for the physics world (unused unless scenes override) |
| `with_fullscreen(enabled)` | `boolean` | `false` | Fullscreen mode |
| `with_sentry(dsn)` | `string` | `""` | Sentry crash reporting DSN. Pass empty string to disable. |
| `with_ticks(count)` | `integer (0-255)` | `0` | Tick rate per second. `0` disables ticking. When enabled, `on_tick(tick)` fires at this rate. |

### create()

```lua
engine = factory:create()
```

Returns a shared engine instance. **Side effects** — sets these globals:

| Global | Type | Description |
|--------|------|-------------|
| `overlay` | `Overlay` | The overlay system (cursor + labels) |
| `scenemanager` | `SceneManager` | The scene lifecycle manager |
| `viewport` | `table` | `{ width = <float>, height = <float> }` — logical resolution |
| `mouse` | `table` | Mouse state queries (see [Global Instances](#4-global-instances)) |

### Typical Usage

```lua
engine = EngineFactory.new()
  :with_title("My Game")
  :with_width(1920)
  :with_height(1080)
  :with_scale(4.0)
  :with_fullscreen(true)
  :with_ticks(10)
  :create()

function setup()
  scenemanager:register("myscene")
  scenemanager:set("myscene")
end
```

---

## 3. Global Functions

These functions are available in all Lua scripts.

### `_(key)` — Localization

```lua
local text = _("greeting")
```

Looks up `key` in the loaded locale file (`locales/<lang>.json`). The language is auto-detected from the OS. Returns the translated string, or the key itself if not found.

### `moment()` — Current Time

```lua
local now = moment()  --> integer (milliseconds since SDL init)
```

### `openurl(url)` — Open External URL

```lua
openurl("https://example.com")
```

Opens the URL in the default browser. On web builds, opens a new tab.

### `queryparam(key, default)` — Query Parameter / Environment Variable

```lua
local scene = queryparam("scene", "mainmenu")
```

- **Web builds**: reads from `URLSearchParams` (e.g., `?scene=test`).
- **Native builds**: reads the uppercase environment variable (e.g., `SCENE=test`).
- Returns `default` if not found.

### `sentinel(table, name)` — Scene Module Registration

```lua
sentinel(scene, "myscene")
```

Registers a sentinel garbage-collection tracker on the scene table. This MUST be called at the end of every scene module, before `return scene`. It enables the engine to track scene lifecycle and cleanup.

---

## 4. Global Instances

These globals are set by the engine and available in all scripts after `EngineFactory:create()` is called.

### `cassette` — Persistent Save System

Type: `Cassette`. See [Cassette](#17-cassette-persistent-save).

Available immediately (before `create()`). A global instance for key-value persistent storage.

### `keyboard` — Keyboard State

Type: `Keyboard`. See [Keyboard](#24-keyboard).

### `gamepads` — Gamepad State

Type: `Gamepads`. See [Gamepad](#25-gamepad).

### `viewport` — Logical Resolution

```lua
viewport.width   --> float
viewport.height  --> float
```

Set after `create()`. Read-only table.

### `mouse` — Mouse State

```lua
mouse.x       --> float (current X in logical coordinates)
mouse.y       --> float (current Y in logical coordinates)
mouse.xy      --> float, float (both coordinates)
mouse.button  --> integer (MouseButton enum value: 0=none, 1=left, 2=middle, 3=right)
```

Set after `create()`. All are functions, not properties.

### `overlay` — Overlay System

Type: `Overlay`. See [Overlay](#14-overlay). Set after `create()`.

### `scenemanager` — Scene Manager

Type: `SceneManager`. See [SceneManager](#8-scenemanager). Set after `create()`.

### `pool` — Per-Scene Object Pool

Type: `table`. Set during each scene's `on_enter()` callback. Contains all named objects and sounds from the current scene, keyed by name.

```lua
pool["myobject"]   --> Entity instance
pool["mysound"]    --> SoundFX instance
pool["myparticle"] --> ParticleProps instance
```

Also supports dynamic key-value storage via direct assignment:

```lua
pool.counter = 0
pool.counter = pool.counter + 1
```

### `world` — Per-Scene Physics World

Type: `table`. Set during each scene's `on_enter()` callback. Contains physics query functions.

```lua
world.raycast(origin, angle, distance, mask)
```

See [World](#26-world-per-scene-physics).

---

## 5. Math Overrides

The engine overrides `math.random` and `math.randomseed` with its own xorshift128+ PRNG.

### `math.random()`

```lua
math.random()              --> float in [0, 1)
math.random(upper)         --> integer in [1, upper]
math.random(low, high)     --> integer in [low, high]
```

### `math.randomseed(seed)`

```lua
math.randomseed(42)  -- seeds the script PRNG with integer value
```

---

## 6. Types (Usertypes)

### 6.1 Vec2

2D vector. Used for positions, velocities, directions.

#### Constructors

```lua
local v = Vec2.new()         --> {x=0, y=0}
local v = Vec2.new(3.0, 4.0) --> {x=3, y=4}
```

#### Fields (read/write)

| Field | Type |
|-------|------|
| `x` | `float` |
| `y` | `float` |

#### Static Factory Methods

| Method | Returns |
|--------|---------|
| `Vec2.zero()` | `Vec2(0, 0)` |
| `Vec2.one()` | `Vec2(1, 1)` |
| `Vec2.up()` | `Vec2(0, -1)` |
| `Vec2.down()` | `Vec2(0, 1)` |
| `Vec2.left()` | `Vec2(-1, 0)` |
| `Vec2.right()` | `Vec2(1, 0)` |

#### Static Methods

| Method | Signature | Returns |
|--------|-----------|---------|
| `Vec2.lerp(a, b, t)` | `(Vec2, Vec2, float) -> Vec2` | Linear interpolation |
| `Vec2.dot(a, b)` | `(Vec2, Vec2) -> float` | Dot product |
| `Vec2.cross(a, b)` | `(Vec2, Vec2) -> float` | 2D cross product (scalar) |
| `Vec2.length(v)` | `(Vec2) -> float` | Magnitude |
| `Vec2.length_squared(v)` | `(Vec2) -> float` | Squared magnitude |
| `Vec2.distance(a, b)` | `(Vec2, Vec2) -> float` | Distance between two points |
| `Vec2.distance_squared(a, b)` | `(Vec2, Vec2) -> float` | Squared distance |
| `Vec2.normalize(v)` | `(Vec2) -> Vec2` | Unit vector |
| `Vec2.angle(v)` | `(Vec2) -> float` | Angle in radians |
| `Vec2.angle_between(a, b)` | `(Vec2, Vec2) -> float` | Angle between two vectors |
| `Vec2.rotate(v, radians)` | `(Vec2, float) -> Vec2` | Rotate vector |
| `Vec2.perpendicular(v)` | `(Vec2) -> Vec2` | Perpendicular vector |
| `Vec2.reflect(v, normal)` | `(Vec2, Vec2) -> Vec2` | Reflect across normal |
| `Vec2.project(v, onto)` | `(Vec2, Vec2) -> Vec2` | Project onto vector |
| `Vec2.clamp(v, min, max)` | `(Vec2, Vec2, Vec2) -> Vec2` | Clamp components |

#### Operators

| Operator | Description |
|----------|-------------|
| `a + b` | Component-wise addition |
| `a - b` | Component-wise subtraction |
| `a * b` | Component-wise multiplication (Vec2 * Vec2) |
| `a * s` | Scalar multiplication (Vec2 * float) |
| `a / s` | Scalar division (Vec2 / float) |
| `-a` | Negation |
| `a == b` | Equality |

---

### 6.2 Vec3

3D vector. Limited use.

#### Constructor

```lua
local v = Vec3.new()               --> {x=0, y=0, z=0}
local v = Vec3.new(1.0, 2.0, 3.0)  --> {x=1, y=2, z=3}
```

#### Fields (read/write)

| Field | Type |
|-------|------|
| `x` | `float` |
| `y` | `float` |
| `z` | `float` |

---

### 6.3 Quad

Rectangle. Used for sprite regions, viewports, hitboxes.

#### Constructor

```lua
local q = Quad.new()                      --> {x=0, y=0, width=0, height=0}
local q = Quad.new(10, 20, 100, 50)       --> {x=10, y=20, width=100, height=50}
```

#### Fields (read/write)

| Field | Type |
|-------|------|
| `x` | `float` |
| `y` | `float` |
| `width` | `float` |
| `height` | `float` |

---

### 6.4 Color

RGBA color. Constructed from hex strings.

#### Constructor

```lua
local c = Color.color("#FF0000")     --> red, alpha=255
local c = Color.color("#FF000080")   --> red, alpha=128
```

Accepts `#RRGGBB` or `#RRGGBBAA` hex format.

#### Properties (read-only)

| Property | Type |
|----------|------|
| `r` | `integer (0-255)` |
| `g` | `integer (0-255)` |
| `b` | `integer (0-255)` |
| `a` | `integer (0-255)` |

#### Operators

| Operator | Description |
|----------|-------------|
| `a == b` | Equality |

---

## 7. Enums

### 7.1 Flip

Controls sprite rendering flip.

| Value | Description |
|-------|-------------|
| `Flip.none` | No flipping |
| `Flip.horizontal` | Mirror horizontally |
| `Flip.vertical` | Mirror vertically |
| `Flip.both` | Mirror both axes |

### 7.2 KeyEvent

Keyboard key codes passed to `on_keypress` and `on_keyrelease`.

| Value | Key |
|-------|-----|
| `KeyEvent.up` | Arrow Up |
| `KeyEvent.left` | Arrow Left |
| `KeyEvent.down` | Arrow Down |
| `KeyEvent.right` | Arrow Right |
| `KeyEvent.space` | Space |
| `KeyEvent.backspace` | Backspace |
| `KeyEvent.enter` | Enter/Return |
| `KeyEvent.escape` | Escape |

### 7.3 MouseButton

Mouse button identifiers.

| Value | Button |
|-------|--------|
| `MouseButton.none` | No button (0) |
| `MouseButton.left` | Left button (1) |
| `MouseButton.middle` | Middle button (2) |
| `MouseButton.right` | Right button (3) |

### 7.4 Player

Gamepad slot indices.

| Value | Slot |
|-------|------|
| `Player.one` | 0 |
| `Player.two` | 1 |
| `Player.three` | 2 |
| `Player.four` | 3 |

### 7.5 PhysicsCategory

Bitmask categories for physics filtering. Available per-scene (set during `on_enter`).

| Value | Bit | Description |
|-------|-----|-------------|
| `PhysicsCategory.none` | `0` | No category |
| `PhysicsCategory.player` | `1 << 0` | Player entities |
| `PhysicsCategory.enemy` | `1 << 1` | Enemy entities |
| `PhysicsCategory.projectile` | `1 << 2` | Projectiles |
| `PhysicsCategory.terrain` | `1 << 3` | Static terrain |
| `PhysicsCategory.trigger` | `1 << 4` | Trigger zones |
| `PhysicsCategory.collectible` | `1 << 5` | Collectible items |
| `PhysicsCategory.interface` | `1 << 6` | UI elements |
| `PhysicsCategory.all` | `~0` | All categories |

---

## 8. SceneManager

Manages scene loading, switching, and destruction.

### Properties

| Property | Type | Access | Description |
|----------|------|--------|-------------|
| `current` | `string` | read-only | Name of the currently active scene |

### Methods

#### `scenemanager:register(name)`

Loads and registers a scene from `scenes/<name>.json` and `scenes/<name>.lua`. The scene is parsed, its assets are loaded, and its Lua module is executed — but the scene does NOT become active until `set()` is called.

```lua
scenemanager:register("mainmenu")
```

#### `scenemanager:set(name)`

Switches to a previously registered scene. Triggers `on_leave()` on the current scene and `on_enter()` on the new scene. The transition is deferred to the next update cycle.

```lua
scenemanager:set("mainmenu")
```

#### `scenemanager:destroy(name)`

Destroys one or more loaded scenes, freeing their resources.

```lua
scenemanager:destroy("mainmenu")   -- destroy specific scene
scenemanager:destroy("*")          -- destroy all scenes except the current one
```

**Important**: Also clears the scene's Lua module from `package.loaded` so `require` will reload it next time.

---

## 9. Scene Module Callbacks

Each scene is a Lua module that returns a table. The engine extracts named callback functions from this table.

### Required Pattern

```lua
local scene = {}

function scene.on_enter()
  -- called when this scene becomes active
  -- pool, world, and PhysicsCategory are available here
end

sentinel(scene, "scenename")  -- MUST call before return
return scene
```

### Callback Reference

| Callback | Signature | When Called |
|----------|-----------|------------|
| `on_enter()` | `() -> void` | **Required**. Scene becomes active. `pool` global is populated with all objects, sounds, and particles from the scene. |
| `on_leave()` | `() -> void` | Scene is being deactivated. Clean up timers, subscriptions, etc. |
| `on_loop(delta)` | `(float) -> void` | Every frame. `delta` is seconds since last frame. Use for animations, tween updates, etc. |
| `on_tick(tick)` | `(integer) -> void` | At the tick rate set by `with_ticks()`. `tick` cycles from 1 to `ticks`. |
| `on_touch(x, y)` | `(float, float) -> void` | Mouse click/tap on the background (not on any object with a hitbox). Coordinates in logical space. |
| `on_motion(x, y)` | `(float, float) -> void` | Mouse/pointer movement. Coordinates in logical space. |
| `on_keypress(code)` | `(integer) -> void` | Key pressed. `code` matches `KeyEvent` enum values. |
| `on_keyrelease(code)` | `(integer) -> void` | Key released. `code` matches `KeyEvent` enum values. |
| `on_text(text)` | `(string) -> void` | Text input event (for text fields, chat, etc.). |
| `on_camera(delta)` | `(float) -> Quad` | **Tilemap scenes only**. Must return a `Quad` representing the camera viewport position and size. Called every frame. |

### Callback Execution Order Per Frame

1. `on_camera(delta)` — if tilemap scene
2. Animation system update
3. Velocity system update
4. Physics system update
5. Sound system update
6. Particle system update
7. Script system update (calls per-object `on_loop`)
8. Render system update
9. `on_loop(delta)`

### Scene Decorators

Scenes can be wrapped/decorated before returning:

```lua
-- Add tick-based timers (wraps on_tick, on_leave)
ticker.wrap(scene)

-- Add HUD overlay (wraps on_enter, on_leave, on_motion)
HUD(scene, { layout = "layout", character = "boy", items = {"item1", "item2"} })

-- MUST be last before return
sentinel(scene, "scenename")
return scene
```

---

## 10. Entity (Object Proxy)

The `Entity` usertype is the Lua representation of a game object. Entities are accessed via the `pool` table by their `name` from the scene JSON.

```lua
local player = pool["player"]   -- or pool.player
```

### Properties

| Property | Type | Access | Description |
|----------|------|--------|-------------|
| `id` | `integer` | read-only | Unique entity ID (uint64) |
| `x` | `float` | read/write | X position |
| `y` | `float` | read/write | Y position |
| `z` | `integer` | read/write | Z-order (draw order). Lower = behind, higher = in front |
| `alpha` | `integer (0-255)` | read/write | Opacity. 0 = invisible, 255 = fully opaque |
| `angle` | `float` | read/write | Rotation in degrees |
| `scale` | `float` | read/write | Scale factor (default 1.0) |
| `flip` | `Flip` | read/write | Sprite flip mode |
| `visible` | `boolean` | read/write | Whether the entity is rendered |
| `action` | `string` | read/write | Current animation timeline name. Changing this resets the animation. |
| `kind` | `string` | read/write | Entity kind identifier (maps to the object type) |
| `position` | `Vec2` | read/write | Position as Vec2. Setter accepts `{x, y}` or `{x=N, y=N}` table |
| `velocity` | `Vec2` | read/write | Velocity as Vec2. Setter accepts `{x, y}` or `{x=N, y=N}` table |
| `alive` | `boolean` | read-only | Whether the entity still exists in the registry |

#### Setting position and velocity with tables

```lua
entity.position = { 100, 200 }           -- positional {x, y}
entity.position = { x = 100, y = 200 }   -- named {x, y}
entity.velocity = { x = 50, y = -30 }
```

### Callback Setters

These set callback functions on the entity. They can be called from scene scripts or object scripts.

| Setter | Signature | When Called |
|--------|-----------|------------|
| `entity:on_hover(fn)` | `fn()` | Mouse enters the entity's hitbox |
| `entity:on_unhover(fn)` | `fn()` | Mouse leaves the entity's hitbox |
| `entity:on_touch(fn)` | `fn(x, y)` | Mouse click/tap on the entity's hitbox. `x`, `y` are logical coordinates. |
| `entity:on_begin(fn)` | `fn(action)` | Animation timeline begins playing. `action` is the timeline name. |
| `entity:on_end(fn)` | `fn(action)` | Animation timeline finishes (oneshot complete or loop restart). `action` is the timeline name. |
| `entity:on_collision(fn)` | `fn(id, kind)` | Physics collision begins. `id` is the other entity's ID, `kind` is its kind string. |
| `entity:on_collision_end(fn)` | `fn(id, kind)` | Physics collision ends. |
| `entity:on_tick(fn)` | `fn(tick)` | Per-tick update (at the engine tick rate). `tick` is the current tick number. |
| `entity:on_screen_exit(fn)` | `fn()` | Entity moves completely outside the camera viewport |
| `entity:on_screen_enter(fn)` | `fn()` | Entity re-enters the camera viewport |
| `entity:on_appear(fn)` | `fn(action)` | Entity's action changes to a timeline that has frames (becomes visible in animation). |
| `entity:on_disappear(fn)` | `fn()` | Entity's action changes to a timeline that has no frames (becomes invisible in animation). |

### Methods

#### `entity:clone()`

Creates a deep copy of the entity with all components (transform, sprite, animation, physics, script).

```lua
local copy = entity:clone()
copy.position = { 100, 200 }
```

The clone gets an auto-incremented name suffix (e.g., `"projectile_1"`, `"projectile_2"`). If the original has a Lua script, the clone gets its own script environment with `self` bound to the new entity.

Returns: `Entity`

#### `entity:die()`

Destroys the entity immediately. After this call, `entity.alive` returns `false`.

```lua
entity:die()
```

#### `entity:observable(name)`

Returns the `Observable` for a named property on this entity.

```lua
local obs = entity:observable("health")
```

Returns: `Observable`

#### `entity:subscribe(name, fn)`

Subscribes to changes on a named property. Shorthand for `entity:observable(name):subscribe(fn)`.

```lua
local id = entity:subscribe("health", function(value)
  print("Health changed to", value)
end)
```

Returns: `integer` (subscription ID)

#### `entity:unsubscribe(name, id)`

Removes a subscription by ID.

```lua
entity:unsubscribe("health", id)
```

### Dynamic Properties (Metatable Behavior)

Entities support dynamic property access via `__index` and `__newindex` metamethods.

#### Reading (`entity.foo`)

1. First checks if the entity has a `scriptable` component with a module containing `on_foo` function. If found, returns a callable wrapper.
2. Otherwise, returns the value from the entity's key-value store (`observable:value()`).

#### Writing (`entity.foo = value`)

Sets the value in the entity's key-value store. This triggers all subscribers on the `"foo"` observable.

```lua
-- In scene script:
pool.enemy.health = 100              -- sets observable "health" to 100
print(pool.enemy.health)             -- reads observable "health" value
pool.enemy.animate()                 -- calls on_animate from the object's script module
```

---

## 11. Object Script Module Callbacks

Each object can have a Lua script at `objects/<scenename>/<kind>.lua`. This script returns a table of callback functions. The engine automatically wires these callbacks to the entity's ECS components.

### Required Pattern

```lua
return {
  on_spawn = function()
    -- self is automatically bound to this entity
  end,
}
```

### The `self` Variable

Inside object scripts, `self` is automatically available and refers to the entity (`Entity` instance). You do NOT need to pass it — the engine injects it into the script's environment.

```lua
return {
  on_spawn = function()
    self.alpha = 128       -- self is the entity
    pool.other.x = 100     -- pool is also available
  end,
}
```

### Callback Reference

| Callback | Signature | When Called |
|----------|-----------|------------|
| `on_spawn()` | `() -> void` | Entity is created and the scene enters. Called once per scene activation. |
| `on_dispose()` | `() -> void` | Scene is leaving. Called once per scene deactivation. |
| `on_loop(delta)` | `(float) -> void` | Every frame. `delta` is seconds since last frame. |
| `on_hover()` | `() -> void` | Mouse enters the entity's hitbox |
| `on_unhover()` | `() -> void` | Mouse leaves the entity's hitbox |
| `on_touch(x, y)` | `(float, float) -> void` | Mouse click/tap on entity. Coordinates in logical space. |
| `on_begin(action)` | `(string) -> void` | Animation timeline begins. `action` is the timeline name. |
| `on_end(action)` | `(string) -> void` | Animation timeline finishes. |
| `on_collision(id, kind)` | `(integer, string) -> void` | Physics collision begins. `id` is entity ID, `kind` is kind string. |
| `on_collision_end(id, kind)` | `(integer, string) -> void` | Physics collision ends. |
| `on_screen_exit()` | `() -> void` | Entity exits camera viewport |
| `on_screen_enter()` | `() -> void` | Entity enters camera viewport |
| `on_appear(action)` | `(string) -> void` | Entity becomes visible (action changed to a timeline with frames). |
| `on_disappear()` | `() -> void` | Entity becomes invisible (action changed to a timeline without frames). |

### Accessing Object Callbacks From Scene Scripts

Object script callbacks prefixed with `on_` can be called from scene scripts via the entity's dynamic property access:

```lua
-- If the object script defines on_animate:
--   return { on_animate = function() ... end }
-- Then from the scene script:
pool.myobject.animate()  -- calls on_animate (the "on_" prefix is stripped)
```

---

## 12. Observable

Reactive value container with subscription support. Created automatically for entity dynamic properties.

### Properties

| Property | Type | Access | Description |
|----------|------|--------|-------------|
| `value` | `any` | read-only | Current stored value |

### Methods

#### `observable:set(value)`

Sets the value and notifies all subscribers.

```lua
local obs = entity:observable("score")
obs:set(42)
```

#### `observable:subscribe(fn)` -> `integer`

Registers a callback that fires whenever the value changes.

```lua
local id = obs:subscribe(function(new_value)
  print("New value:", new_value)
end)
```

Returns: subscription ID (integer)

#### `observable:unsubscribe(id)`

Removes a subscription.

```lua
obs:unsubscribe(id)
```

### Arithmetic Metamethods

Observables support arithmetic operations that extract their numeric value:

| Operation | Result |
|-----------|--------|
| `observable + number` | `value + number` |
| `observable - number` | `value - number` |
| `observable * number` | `value * number` |
| `observable / number` | `value / number` |
| `observable % number` | `value % number` |
| `-observable` | `-value` |
| `observable == value` | Type-safe equality comparison |
| `observable < number` | `value < number` |

If the observable's value is nil/invalid, `0.0` is used as default for arithmetic.

---

## 13. SoundFX

Sound effect instances. Loaded from `.opus` files. Accessed via `pool`:

```lua
pool["mysound"]:play()
```

Sounds are defined in the scene JSON `"sounds"` array. Each entry maps to `blobs/<scenename>/<name>.opus`.

### Constructor

No constructor. Instances are created by the engine from scene JSON.

### Methods

#### `sound:play(loop)`

```lua
sound:play()       -- play once
sound:play(true)   -- loop continuously
sound:play(false)  -- play once (explicit)
```

#### `sound:stop()`

```lua
sound:stop()
```

### Properties

| Property | Type | Access | Description |
|----------|------|--------|-------------|
| `volume` | `float (0.0-1.0)` | read/write | Playback volume. Clamped to [0, 1]. |

### Callback Setters

| Setter | Signature | When Called |
|--------|-----------|------------|
| `sound.on_begin = fn` | `fn()` | Playback starts |
| `sound.on_end = fn` | `fn()` | Playback finishes (for non-looping sounds) |

---

## 14. Overlay

The overlay system renders on top of everything (above all scenes). It manages cursors and text labels.

### Properties

| Property | Type | Access | Description |
|----------|------|--------|-------------|
| `cursor` | `Cursor or string or nil` | read/write | Set to a cursor resource name (string) to create a cursor, or `nil` to remove it. Reading returns the `Cursor` instance. |

### Methods

#### `overlay:label(font)` — Create Label

```lua
local lbl = overlay:label("rpgfont")
```

Creates a new `Label` using the named font. Returns: `Label`

#### `overlay:label(instance)` — Remove Label

```lua
overlay:label(lbl)  -- removes the label
```

Passing an existing `Label` instance removes it from the overlay.

#### `overlay:dispatch(message)`

Sends a message to the cursor (queues a cursor animation action).

```lua
overlay:dispatch("damage")
```

---

## 15. Label

Text rendering widget. Created via `overlay:label(font)`. Inherits from `Widget` (abstract base class with no user-accessible methods).

### Methods

#### `label:set(text, x, y)`

Sets the text content and position.

```lua
label:set("Hello World", 10, 20)
```

#### `label:set(x, y)`

Repositions without changing text.

```lua
label:set(50, 60)
```

#### `label:clear()`

Clears the text, position, and all effects.

```lua
label:clear()
```

### Properties

| Property | Type | Access | Description |
|----------|------|--------|-------------|
| `glyphs` | `string` | read-only | The set of characters supported by this label's font |
| `effect` | `table or nil` | write-only | Per-glyph visual effects (see below) |

### Per-Glyph Effects

Set effects on individual characters by 1-based index:

```lua
label.effect = {
  [1] = { xoffset = 0, yoffset = -2, scale = 1.5, r = 255, g = 0, b = 0, alpha = 255 },
  [2] = { yoffset = 2 },
  [5] = nil,  -- remove effect on character 5
}
```

Set `effect = nil` to clear all effects.

#### Effect Properties

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `xoffset` | `float` | `0.0` | Horizontal offset in pixels |
| `yoffset` | `float` | `0.0` | Vertical offset in pixels |
| `scale` | `float` | `1.0` | Scale factor |
| `r` | `integer (0-255)` | `255` | Red channel |
| `g` | `integer (0-255)` | `255` | Green channel |
| `b` | `integer (0-255)` | `255` | Blue channel |
| `alpha` | `integer (0-255)` | `255` | Opacity |

---

## 16. Cursor

Custom animated cursor. Created via `overlay.cursor = "name"`.

### Properties/Methods

| Method | Signature | Description |
|--------|-----------|-------------|
| `cursor:visible(bool)` | `(boolean) -> void` | Show or hide the cursor |

The cursor automatically:
- Hides the OS cursor when created
- Shows the OS cursor when destroyed
- Plays animation on left/right mouse click (triggers `"left"` / `"right"` animation)
- Returns to `"default"` animation after oneshot completes

---

## 17. Cassette (Persistent Save)

Key-value persistent storage. Data is saved automatically after every `set()` call.

- **Native builds**: saves to `cassette.tape` file in the working directory
- **Web builds**: saves to `localStorage`

### Methods

#### `cassette:set(key, value)`

```lua
cassette:set("player_name", "Hero")
cassette:set("score", 9001)
cassette:set("hardcore", true)
cassette:set("ratio", 3.14)
cassette:set("cleared", nil)  -- stores null
```

Supported value types: `string`, `number` (integer or float), `boolean`, `nil`.

#### `cassette:get(key, fallback)`

```lua
local name = cassette:get("player_name", "Unknown")   --> string
local score = cassette:get("score", 0)                 --> integer
local ratio = cassette:get("ratio", 1.0)               --> float
local hc = cassette:get("hardcore", false)              --> boolean
local val = cassette:get("nonexistent", nil)            --> nil
```

The `fallback` parameter determines the return type. If the key exists, its value is returned (with type coercion for numbers). If not found, `fallback` is returned.

#### `cassette:clear(key)`

Removes a specific key.

```lua
cassette:clear("score")
```

#### `cassette:clear()`

Removes ALL saved data.

```lua
cassette:clear()
```

---

## 18. ParticleProps

Runtime properties for a particle emitter instance. Accessed via `pool`:

```lua
pool["myparticle"].spawning = false
pool["myparticle"].position = { x = 100, y = 200 }
```

### Properties

| Property | Type | Access | Description |
|----------|------|--------|-------------|
| `spawning` | `boolean` | read/write | Whether the emitter is actively spawning new particles |
| `position` | `table {x, y}` | write-only | Emitter center position. Accepts `{x=N, y=N}` or `{N, N}`. |

---

## 19. ParticlePool

Global particle pool management. The pool is per-scene but the type is exposed via `pool`.

### Methods

#### `particlepool:clear()`

Destroys all particle emitters and their entities.

> Note: This is exposed as a type method but is typically called on the pool's particle manager, not on individual particles.

---

## 20. Achievement

Steam achievement integration. Available as global `achievement`.

### Methods

#### `achievement:unlock(id)`

Unlocks a Steam achievement by its API name.

```lua
achievement:unlock("ACH_FIRST_BLOOD")
```

No-op if Steam is unavailable or achievement already unlocked.

---

## 21. User & Buddy

Steam social integration. Available as global `user`.

### User Properties

| Property | Type | Access | Description |
|----------|------|--------|-------------|
| `persona` | `string` | read-only | Steam display name. Returns `""` if Steam unavailable. |
| `buddies` | `table of Buddy` | read-only | List of Steam friends. Returns `{}` if Steam unavailable. |

### Buddy Properties

| Property | Type | Access | Description |
|----------|------|--------|-------------|
| `id` | `integer` | read-only | Steam friend ID (uint64) |
| `name` | `string` | read-only | Friend's display name |

```lua
print(user.persona)
for _, buddy in ipairs(user.buddies) do
  print(buddy.id, buddy.name)
end
```

---

## 22. OperatingSystem

System information. Available as global `operatingsystem`.

### Properties

| Property | Type | Access | Description |
|----------|------|--------|-------------|
| `compute` | `integer` | read-only | Number of logical CPU cores |
| `memory` | `integer` | read-only | System RAM in megabytes |
| `name` | `string` | read-only | Platform name (e.g., `"Windows"`, `"macOS"`, `"Linux"`, `"Emscripten"`) |

```lua
print(operatingsystem.name)      --> "macOS"
print(operatingsystem.compute)   --> 8
print(operatingsystem.memory)    --> 16384
```

---

## 23. Desktop

Desktop file system access. Available as global `desktop`.

### Properties

| Property | Type | Access | Description |
|----------|------|--------|-------------|
| `folder` | `string or nil` | read-only | Path to the user's desktop folder. Returns `nil` if unavailable. |

```lua
local path = desktop.folder
if path then
  print("Desktop at:", path)
end
```

---

## 24. Keyboard

Real-time keyboard state polling. Available as global `keyboard`.

### Usage

Access keys by name via indexing. Returns `true` if the key is currently pressed, `false` otherwise.

```lua
if keyboard.space then
  -- space is held down
end

if keyboard.left then
  -- left arrow is held
end
```

### Supported Key Names

| Key Name | Physical Key |
|----------|-------------|
| `a` - `z` | Letter keys A-Z |
| `0` - `9` | Number keys 0-9 |
| `up` | Arrow Up |
| `down` | Arrow Down |
| `left` | Arrow Left |
| `right` | Arrow Right |
| `shift` | Left Shift |
| `ctrl` | Left Control |
| `escape` | Escape |
| `space` | Space |
| `enter` | Enter/Return |
| `backspace` | Backspace |
| `tab` | Tab |

Returns `nil` for unrecognized key names.

---

## 25. Gamepad

Gamepad input. Available as global `gamepads` (type: `Gamepads`).

### Accessing Gamepads

```lua
local pad = gamepads[Player.one]    -- or gamepads[0]
```

Index with `Player` enum values (0-3) or integer slot numbers. Returns a `GamepadSlot` instance.

### GamepadSlot Properties

All properties below are accessed on a `GamepadSlot` instance (e.g., `gamepads[Player.one].connected`).

| Property | Type | Description |
|----------|------|-------------|
| `connected` | `boolean` | Whether a gamepad is connected in this slot |
| `name` | `string or nil` | Gamepad device name, or `nil` if disconnected |

### Analog Sticks

```lua
local lx, ly = pad.leftstick     -- returns two int16 values with deadzone applied
local rx, ry = pad.rightstick     -- returns two int16 values with deadzone applied
```

Returns `(int16, int16)`. Values range from -32768 to 32767. Deadzone of 8000 is applied (values below threshold return 0).

### Triggers

```lua
local lt, rt = pad.triggers     -- returns two int16 values with deadzone applied
```

### Button State

Access buttons by name. Returns `true`/`false`.

| Button Name | Physical Button |
|-------------|-----------------|
| `south` | A / Cross |
| `east` | B / Circle |
| `west` | X / Square |
| `north` | Y / Triangle |
| `back` | Back / Select |
| `guide` | Guide / Home |
| `start` | Start / Options |
| `leftstick` | Left Stick Click |
| `rightstick` | Right Stick Click |
| `leftshoulder` | Left Bumper (LB) |
| `rightshoulder` | Right Bumper (RB) |
| `up` | D-Pad Up |
| `down` | D-Pad Down |
| `left` | D-Pad Left |
| `right` | D-Pad Right |

```lua
if pad.south then
  -- A/Cross is pressed
end
```

### Raw Axis Values

| Axis Name | Description |
|-----------|-------------|
| `leftx` | Left stick X axis |
| `lefty` | Left stick Y axis |
| `rightx` | Right stick X axis |
| `righty` | Right stick Y axis |
| `triggerleft` | Left trigger axis |
| `triggerright` | Right trigger axis |

Returns `int16` with deadzone applied.

### Global Properties

```lua
gamepads.count   --> integer (number of connected gamepads, max 4)
```

---

## 26. World (Per-Scene Physics)

Physics query interface. Available as `world` during scene callbacks.

### `world.raycast(origin, angle, distance [, mask])`

Casts a ray from `origin` at `angle` degrees for `distance` pixels. Returns a table of `Entity` instances hit by the ray, sorted by distance (nearest first).

**Overload 1** — Vec2 origin:

```lua
local hits = world.raycast(Vec2.new(100, 100), 45.0, 200.0)
local hits = world.raycast(Vec2.new(100, 100), 45.0, 200.0, PhysicsCategory.enemy)
```

**Overload 2** — Separate x, y:

```lua
local hits = world.raycast(100, 100, 45.0, 200.0)
local hits = world.raycast(100, 100, 45.0, 200.0, PhysicsCategory.enemy)
```

**Parameters**:

| Parameter | Type | Description |
|-----------|------|-------------|
| `origin` / `x, y` | `Vec2` or `float, float` | Ray start position |
| `angle` | `float` | Direction in degrees |
| `distance` | `float` | Maximum ray length in pixels |
| `mask` | `PhysicsCategory` (optional) | Category filter. Default: `PhysicsCategory.all` |

**Returns**: `table` of `Entity` (sorted by distance, nearest first)

---

## 27. JSON Schemas

### 27.1 Scene JSON — `scenes/<name>.json`

```jsonc
{
  "width": 480,                              // float — scene world width
  "height": 270,                             // float — scene world height
  "layer": {
    "type": "background"                     // "background" or "tilemap"
    // If "tilemap":
    // "content": "tilemapname"              // references tilemaps/<name>.json
  },
  "sounds": ["sound1", "sound2"],            // optional — each loads blobs/<scene>/<name>.opus
  "fonts": ["rpgfont"],                      // optional — each preloads fonts/<name>.json
  "physics": {                               // optional — per-scene physics config
    "gravity": { "x": 0.0, "y": 9.8 }
  },
  "objects": [                               // optional — list of entities to spawn
    {
      "kind": "objecttype",                  // REQUIRED — references objects/<scene>/<kind>.json
      "name": "instancename",               // REQUIRED — key in pool table
      "action": "default",                   // optional — initial animation timeline
      "x": 100,                              // optional — initial X position (default 0)
      "y": 200,                              // optional — initial Y position (default 0)
      "type": "particle",                    // optional — "particle" for particle emitters, omit for objects
      "spawning": true                       // optional — particle-specific, default true
    }
  ]
}
```

**Layer types**:
- `"background"` — renders `blobs/<scenename>/background.png` as the scene background. The camera is fixed to `width` x `height`.
- `"tilemap"` — renders a tilemap from `tilemaps/<content>.json`. Camera is controlled by `on_camera()`.

---

### 27.2 Object JSON — `objects/<scenename>/<kind>.json`

Defines sprite animations (timelines) for an object.

```jsonc
{
  "scale": 1.0,                              // optional — default scale factor (default 1.0)
  "timelines": {
    "<action_name>": {                       // action name (e.g., "default", "walk", "idle")
      "oneshot": false,                      // optional — play once then stop (default false)
      "next": "other_action",                // optional — auto-transition to this action after oneshot
      "hitbox": {                            // optional — collision hitbox
        "aabb": {
          "x": 10,                           // float — hitbox X offset from sprite origin
          "y": 20,                           // float — hitbox Y offset
          "w": 32,                           // float — hitbox width
          "h": 48                            // float — hitbox height
        }
      },
      "frames": [                            // array of animation frames
        {
          "duration": 200,                   // integer — frame duration in ms. -1 = static (infinite)
          "offset": {                        // sprite draw offset from entity position
            "x": 10,
            "y": 20
          },
          "quad": {                          // source rectangle in the spritesheet
            "x": 0,                          // float — source X in PNG
            "y": 0,                          // float — source Y in PNG
            "w": 32,                         // float — source width
            "h": 48                          // float — source height
          }
        }
      ]
    }
  }
}
```

**Key rules**:
- The spritesheet image is at `blobs/<scenename>/<kind>.png`.
- An action with no frames (or a missing action) makes the entity invisible in animation.
- `duration: -1` means the frame displays indefinitely (static sprite).
- `oneshot: true` plays the animation once, then transitions to `next` (if specified) or stops.
- The `hitbox.aabb` defines the physics sensor shape for collision/touch detection.

---

### 27.3 Particle JSON — `particles/<kind>.json`

Defines a particle emitter type.

```jsonc
{
  "count": 60,                               // integer — number of particles in the pool
  "spawn": {                                 // optional — spawn parameters
    "x": { "start": -5.0, "end": 5.0 },     // optional — random X offset range from emitter position
    "y": { "start": -5.0, "end": 5.0 },     // optional — random Y offset range
    "radius": { "start": 0.0, "end": 10.0 },// optional — random spawn radius range
    "angle": { "start": 0.0, "end": 360.0 },// optional — random spawn angle range (degrees, used with radius)
    "scale": { "start": 0.5, "end": 2.0 },  // optional — random scale range (default {1.0, 1.0})
    "life": { "start": 0.5, "end": 2.0 }    // optional — random lifetime in seconds (default {1.0, 1.0})
  },
  "velocity": {                              // optional — initial velocity
    "x": { "start": 0.0, "end": 0.0 },      // optional — random X velocity range
    "y": { "start": -30.0, "end": -4.0 }    // optional — random Y velocity range
  },
  "gravity": {                               // optional — acceleration applied per second
    "x": { "start": 0.0, "end": 0.0 },      // optional — random X gravity range
    "y": { "start": 0.0, "end": 0.0 }       // optional — random Y gravity range
  },
  "rotation": {                              // optional — angular motion
    "force": { "start": 0.0, "end": 0.5 },  // optional — random angular acceleration range
    "velocity": { "start": 0.0, "end": 0.0 }// optional — random initial angular velocity range
  }
}
```

**Key rules**:
- Particle texture is at `blobs/particles/<kind>.png`.
- All range values use `{ "start": min, "end": max }` format. A random value within the range is chosen per particle on spawn.
- When a particle's `life` reaches 0, it respawns (if `spawning` is true on the props).
- Particle alpha fades based on remaining life (clamped to [0, 1]).
- Defaults for all range fields are `{0.0, 0.0}` except `scale` `{1.0, 1.0}` and `life` `{1.0, 1.0}`.

---

### 27.4 Font JSON — `fonts/<family>.json`

Minimal font descriptor. The actual glyph parsing is done from the spritesheet pixels.

```jsonc
{
  "glyphs": " abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.,!?-+/():;%&`'*#=[]\"",
  "spacing": 1,       // optional — integer, horizontal spacing between glyphs in pixels (default 0)
  "leading": 2,       // optional — integer, vertical spacing between lines in pixels (default 0)
  "scale": 1.0        // optional — float, glyph scale factor (default 1.0)
}
```

**How font spritesheet parsing works**:
- The spritesheet PNG is at `blobs/overlay/<family>.png`.
- The **top-left pixel** of the PNG is the **separator color**.
- The engine scans left-to-right across the top row. Columns matching the separator color are glyph boundaries.
- Each glyph's width and height are determined by scanning until the next separator pixel.
- Glyphs are mapped 1-to-1 to the characters in the `"glyphs"` string, in order.
- The `"glyphs"` string defines exactly which characters the font supports and in what order they appear in the spritesheet.

---

### 27.5 Cursor JSON — `cursors/<name>.json`

Defines an animated cursor.

```jsonc
{
  "point": {                                 // hotspot — the "click point" offset from top-left
    "x": 18,
    "y": 9
  },
  "animations": {
    "default": {                             // REQUIRED — idle/default animation
      "oneshot": true,                       // optional — play once then loop (default false)
      "frames": [
        {
          "duration": 120,                   // integer — frame duration in ms. 0 = static.
          "offset": {                        // draw offset from cursor position
            "x": 11,
            "y": 7
          },
          "quad": {                          // source rectangle in spritesheet
            "x": 194,
            "y": 139,
            "w": 30,
            "h": 66
          }
        }
      ]
    },
    "left": { ... },                         // optional — played on left mouse click
    "right": { ... },                        // optional — played on right mouse click
    "<custom>": { ... }                      // optional — played via overlay:dispatch("<custom>")
  }
}
```

**Key rules**:
- The spritesheet is at `blobs/overlay/<name>.png`.
- `"default"` animation is required and plays when idle.
- `"left"` and `"right"` animations are triggered by mouse clicks.
- Custom animation names can be triggered via `overlay:dispatch("name")`.
- After a `oneshot` animation completes, the cursor returns to `"default"` or a queued action.

---

### 27.6 Tilemap JSON — `tilemaps/<name>.json`

Defines a tile-based level.

```jsonc
{
  "tile_size": 128,                          // float — size of each tile in pixels
  "width": 120,                              // integer — number of columns
  "height": 18,                              // integer — number of rows
  "layers": [                                // array of tile layers (rendered bottom to top)
    {
      "collider": true,                      // optional — whether this layer generates physics bodies (default false)
      "tiles": [                             // flat array of tile IDs, length = width * height
        0, 0, 0, 23, 25, 36, ...            // 0 = empty, 1+ = tile index in atlas (1-indexed)
      ]
    }
  ]
}
```

**Key rules**:
- The tile atlas spritesheet is at `blobs/tilemaps/<name>.png`.
- Tile IDs in the `tiles` array are **1-indexed**. `0` means empty (no tile).
- Tile ID `N` corresponds to the N-th tile in the atlas (left-to-right, top-to-bottom, 1-based).
- The atlas is divided into tiles of `tile_size` x `tile_size` pixels.
- Layers with `"collider": true` generate static physics bodies. Adjacent solid tiles are merged into larger rectangular bodies for efficiency.
- Camera scrolling for tilemap scenes is controlled by `on_camera(delta)` returning a `Quad`.

---

### 27.7 Localization JSON — `locales/<lang>.json`

Simple key-value string mapping.

```jsonc
{
  "greeting": "Hello, world!",
  "menu_start": "Start Game",
  "menu_quit": "Quit",
  "dialog_intro": "Once upon a time..."
}
```

**Key rules**:
- Language is auto-detected from the OS locale (e.g., `"en"`, `"pt"`, `"es"`).
- The engine loads `locales/<detected_lang>.json`.
- Access via `_("key")` in Lua. Returns the key itself if not found (graceful fallback).

---

## 28. Complete Examples

### 28.1 Minimal `scripts/main.lua`

```lua
engine = EngineFactory.new()
  :with_title("My Game")
  :with_width(1920)
  :with_height(1080)
  :with_scale(4.0)
  :create()

function setup()
  scenemanager:register("intro")
  scenemanager:set("intro")
end
```

### 28.2 Scene with Background — `scenes/intro.json` + `scenes/intro.lua`

**`scenes/intro.json`**:
```json
{
  "width": 480,
  "height": 270,
  "layer": { "type": "background" },
  "sounds": ["click"],
  "objects": [
    {
      "kind": "button",
      "name": "startbutton",
      "action": "normal"
    }
  ]
}
```

**`scenes/intro.lua`**:
```lua
local scene = {}

function scene.on_enter()
  -- pool.startbutton is an Entity
  -- pool.click is a SoundFX
end

function scene.on_touch(x, y)
  pool.click:play()
end

function scene.on_keypress(code)
  if code == KeyEvent.escape then
    -- handle escape
  end
end

sentinel(scene, "intro")
return scene
```

### 28.3 Scene with Tilemap and Camera — `scenes/level1.json` + `scenes/level1.lua`

**`scenes/level1.json`**:
```json
{
  "width": 480,
  "height": 270,
  "layer": { "type": "tilemap", "content": "level1" },
  "objects": [
    {
      "kind": "player",
      "name": "player",
      "action": "idle",
      "x": 100,
      "y": 200
    }
  ]
}
```

**`scenes/level1.lua`**:
```lua
local scene = {}

local camera_x = 0

function scene.on_enter()
  -- set up player controls, etc.
end

function scene.on_camera(delta)
  -- follow the player
  camera_x = pool.player.x - viewport.width / 2
  return Quad.new(camera_x, 0, viewport.width, viewport.height)
end

function scene.on_loop(delta)
  -- update game logic
end

sentinel(scene, "level1")
return scene
```

### 28.4 Object with Hover and Touch — `objects/intro/button.json` + `objects/intro/button.lua`

**`objects/intro/button.json`**:
```json
{
  "timelines": {
    "normal": {
      "hitbox": {
        "aabb": { "x": 10, "y": 5, "w": 80, "h": 30 }
      },
      "frames": [
        {
          "duration": -1,
          "offset": { "x": 10, "y": 5 },
          "quad": { "x": 0, "y": 0, "w": 80, "h": 30 }
        }
      ]
    },
    "hover": {
      "hitbox": {
        "aabb": { "x": 10, "y": 5, "w": 80, "h": 30 }
      },
      "frames": [
        {
          "duration": -1,
          "offset": { "x": 10, "y": 5 },
          "quad": { "x": 80, "y": 0, "w": 80, "h": 30 }
        }
      ]
    }
  }
}
```

**`objects/intro/button.lua`**:
```lua
return {
  on_hover = function()
    self.action = "hover"
  end,

  on_unhover = function()
    self.action = "normal"
  end,

  on_touch = function(x, y)
    scenemanager:register("game")
    scenemanager:set("game")
  end,
}
```

### 28.5 Object with Collision and Velocity — `objects/game/projectile.lua`

```lua
local speed = 80

return {
  on_spawn = function()
    local radians = math.rad(315)
    self.velocity = {
      x = math.cos(radians) * speed,
      y = math.sin(radians) * speed,
    }
  end,

  on_collision = function(id, kind)
    if kind == "enemy" then
      -- handle hit
    end
  end,

  on_screen_exit = function()
    -- respawn at center
    self.position = { x = 240, y = 135 }

    -- random new direction
    local degrees = math.random(0, 35) * 10
    local radians = math.rad(degrees)
    self.velocity = {
      x = math.cos(radians) * speed,
      y = math.sin(radians) * speed,
    }
  end,
}
```

### 28.6 Object with Observable Subscription — `objects/game/boss.lua`

```lua
return {
  on_spawn = function()
    self:subscribe("misses", function(value)
      if value < 6 then
        return
      end

      pool.scream:play()
      self.action = "summon"
      self.misses = 0
    end)

    self:subscribe("health", function(value)
      if value <= 0 then
        self:die()
      end
    end)
  end,
}
```

Usage from scene script:

```lua
function scene.on_touch()
  pool.boss.misses = (pool.boss.misses or 0) + 1
end
```

### 28.7 Scene with Particles — `scenes/campfire.json` + `scenes/campfire.lua`

**`scenes/campfire.json`**:
```json
{
  "width": 480,
  "height": 270,
  "layer": { "type": "background" },
  "objects": [
    {
      "type": "particle",
      "kind": "smoke",
      "name": "campfire_smoke",
      "x": 240,
      "y": 200,
      "spawning": true
    }
  ]
}
```

**`scenes/campfire.lua`**:
```lua
local scene = {}

function scene.on_enter()
  -- pool.campfire_smoke is a ParticleProps instance
end

function scene.on_touch(x, y)
  -- move particle emitter to click position
  pool.campfire_smoke.position = { x = x, y = y }
end

function scene.on_keypress(code)
  if code == KeyEvent.space then
    -- toggle particle spawning
    pool.campfire_smoke.spawning = not pool.campfire_smoke.spawning
  end
end

sentinel(scene, "campfire")
return scene
```

### 28.8 Complete Game Startup with Scene Transitions

**`scripts/main.lua`**:
```lua
engine = EngineFactory.new()
  :with_title("Adventure")
  :with_width(1920)
  :with_height(1080)
  :with_scale(4.0)
  :with_fullscreen(true)
  :with_ticks(10)
  :create()

math.randomseed(42)

-- Load shared modules
require("globals")

function setup()
  overlay.cursor = "pointer"

  scenemanager:register("mainmenu")
  scenemanager:set("mainmenu")
end
```

**`scripts/globals.lua`**:
```lua
-- Shared state management
state = { system = {} }

setmetatable(state.system, {
  __newindex = function(t, k, v)
    cassette:set("system/" .. k, v)
  end,
  __index = function(t, k)
    return cassette:get("system/" .. k, nil)
  end,
})

setmetatable(state, {
  __newindex = function(t, k, v)
    local scene = scenemanager.current
    cassette:set(scene .. "/" .. k, v)
  end,
  __index = function(t, k)
    local scene = scenemanager.current
    return cassette:get(scene .. "/" .. k, nil)
  end,
})

-- Scene transition helper
function transition(options)
  if options.destroy then
    for _, name in ipairs(options.destroy) do
      scenemanager:destroy(name)
    end
  end

  if options.register then
    for _, name in ipairs(options.register) do
      scenemanager:register(name)
    end
  end
end

-- Quick scene jump helper
jump = {}
function jump.to(name)
  return function()
    scenemanager:register(name)
    scenemanager:set(name)
  end
end
```

**`scenes/mainmenu.lua`** (using transitions):
```lua
local scene = {}

function scene.on_enter()
  transition({
    register = { "level1" },
  })
end

function scene.on_touch()
  scenemanager:set("level1")
end

function scene.on_leave()
  -- cleanup
end

sentinel(scene, "mainmenu")
return scene
```

---

## Appendix: Callback Quick Reference

### Scene Callbacks

| Callback | Arguments | Return | Required |
|----------|-----------|--------|----------|
| `on_enter` | none | void | **Yes** |
| `on_leave` | none | void | No |
| `on_loop` | `delta: float` | void | No |
| `on_tick` | `tick: integer` | void | No |
| `on_touch` | `x: float, y: float` | void | No |
| `on_motion` | `x: float, y: float` | void | No |
| `on_keypress` | `code: integer` | void | No |
| `on_keyrelease` | `code: integer` | void | No |
| `on_text` | `text: string` | void | No |
| `on_camera` | `delta: float` | `Quad` | Only for tilemap scenes |

### Object Script Callbacks

| Callback | Arguments | Return |
|----------|-----------|--------|
| `on_spawn` | none | void |
| `on_dispose` | none | void |
| `on_loop` | `delta: float` | void |
| `on_hover` | none | void |
| `on_unhover` | none | void |
| `on_touch` | `x: float, y: float` | void |
| `on_begin` | `action: string` | void |
| `on_end` | `action: string` | void |
| `on_collision` | `id: integer, kind: string` | void |
| `on_collision_end` | `id: integer, kind: string` | void |
| `on_screen_exit` | none | void |
| `on_screen_enter` | none | void |
| `on_appear` | `action: string` | void |
| `on_disappear` | none | void |

### Entity Callback Setters (from scene scripts)

| Setter | Callback Signature |
|--------|--------------------|
| `entity:on_hover(fn)` | `fn()` |
| `entity:on_unhover(fn)` | `fn()` |
| `entity:on_touch(fn)` | `fn(x, y)` |
| `entity:on_begin(fn)` | `fn(action)` |
| `entity:on_end(fn)` | `fn(action)` |
| `entity:on_collision(fn)` | `fn(id, kind)` |
| `entity:on_collision_end(fn)` | `fn(id, kind)` |
| `entity:on_tick(fn)` | `fn(tick)` |
| `entity:on_screen_exit(fn)` | `fn()` |
| `entity:on_screen_enter(fn)` | `fn()` |
| `entity:on_appear(fn)` | `fn(action)` |
| `entity:on_disappear(fn)` | `fn()` |
