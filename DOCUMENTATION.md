# Carimbo Game Engine Documentation

Carimbo is a modern, cross-platform 2D game engine written in C++23 with Lua scripting. The name comes from the Brazilian Portuguese word for "stamp" - reflecting how 2D engines "stamp" sprites onto the screen.

## Key Technologies

- **C++23** with modern features (concepts, ranges, std::format, std::print)
- **Lua** scripting via sol2 (compatible with both PUC Lua and LuaJIT)
- **SDL3** for windowing, rendering, and input
- **Box2D** for physics
- **EnTT** for Entity Component System (ECS)
- **OpenAL** for audio
- **PhysFS** for virtual filesystem (ROM packaging)
- **yyjson/simdjson** for JSON parsing
- **Boost** containers for performance

## Supported Platforms

- Linux, Windows, macOS
- WebAssembly (Emscripten)
- Android, iOS

---

## Table of Contents

1. [Project Structure](#project-structure)
2. [Engine Initialization](#engine-initialization)
3. [Scene System](#scene-system)
4. [Object System](#object-system)
5. [Pool System](#pool-system)
6. [Particle System](#particle-system)
7. [Tilemap System](#tilemap-system)
8. [Cursor Definition](#cursor-definition)
9. [Font Definition](#font-definition)
10. [Input Handling](#input-handling)
11. [Audio System](#audio-system)
12. [Persistent Storage](#persistent-storage)
13. [Localization](#localization)
14. [Canvas](#canvas)
15. [Engine Globals](#engine-globals)
16. [JSON Schemas](#json-schemas)
17. [Types](#types)
18. [Enums](#enums)

---

## Project Structure

```
cartridge.rom (or cartridge/ directory)
├── scripts/
│   └── main.lua              # Entry point - engine initialization
├── scenes/
│   ├── <scene>.json          # Scene configuration
│   └── <scene>.lua           # Scene logic
├── objects/
│   └── <scene>/
│       ├── <object>.json     # Object animations/hitboxes
│       └── <object>.lua      # Object behavior
├── particles/
│   └── <particle>.json       # Particle system definitions
├── tilemaps/
│   └── <tilemap>.json        # Tile-based level data
├── fonts/
│   └── <font>.json           # Font definitions
├── cursors/
│   └── <cursor>.json         # Custom cursor animations
├── blobs/
│   ├── <scene>/
│   │   ├── <object>.png      # Object sprites
│   │   ├── background.png    # Background images
│   │   └── <sound>.ogg       # Sound effects
│   ├── tilemaps/
│   │   └── <tilemap>.png     # Tilemap atlas
│   └── particles/
│       └── <particle>.png    # Particle textures
└── locales/
    ├── en.json               # English localization
    └── pt.json               # Portuguese localization
```

---

## Engine Initialization

The engine is initialized in `scripts/main.lua` using the builder pattern:

```lua
engine = EngineFactory.new()
  :with_title("My Game")
  :with_width(1920)
  :with_height(1080)
  :with_scale(4.0)
  :with_fullscreen(true)
  :with_ticks(10)
  :with_sentry(dsn)
  :create()

function setup()
  overlay:cursor("arrow")
  scenemanager:register("mainmenu")
  scenemanager:set("mainmenu")
end
```

### EngineFactory Methods

All methods return the factory instance for chaining.

| Method | Type | Description |
|--------|------|-------------|
| `:with_title(title)` | string | Window title |
| `:with_width(width)` | int | Resolution width in pixels |
| `:with_height(height)` | int | Resolution height in pixels |
| `:with_scale(scale)` | float | Pixel scaling factor for rendering |
| `:with_gravity(gravity)` | float | Default physics gravity |
| `:with_fullscreen(fullscreen)` | bool | Enable fullscreen mode |
| `:with_ticks(ticks)` | uint8 | Fixed tick rate for `on_tick` callbacks (ticks per second) |
| `:with_sentry(dsn)` | string | Sentry DSN for crash reporting (release builds only) |
| `:create()` | - | Create and return the engine instance |

### What `:create()` Does

Calling `:create()` initializes the following systems:
- Audio device (OpenAL)
- Window with specified dimensions and fullscreen mode
- Renderer with pixel scaling
- Event manager for input handling
- Overlay for UI (labels, cursor)
- Scene manager for scene lifecycle

After `:create()`, the following globals become available:
- `overlay` - UI overlay system
- `scenemanager` - Scene management
- `canvas` - Pixel manipulation framebuffer
- `viewport` - Table with `width` and `height` of the window

### The `setup()` Function

The engine calls `setup()` after initialization. This is where you should:
1. Set the initial cursor (optional)
2. Register your first scene
3. Set the active scene

---

## Scene System

Scenes are the fundamental organizational unit. Each scene has a JSON configuration file and a Lua logic file.

### Scene Lua File

Located at `scenes/<scene>.lua`:

```lua
local scene = {}

function scene.on_enter()
  pool.theme:play(true)
end

function scene.on_loop(delta)
end

function scene.on_tick(tick)
end

function scene.on_leave()
end

function scene.on_camera(delta)
  local player = pool.player
  local camera_x = player.x - viewport.width / 2
  local camera_y = player.y - viewport.height / 2
  return Quad.new(camera_x, camera_y, viewport.width, viewport.height)
end

function scene.on_touch(x, y)
end

function scene.on_motion(x, y)
end

function scene.on_keypress(code)
  if code == KeyEvent.escape then
    scenemanager:set("mainmenu")
  end
end

function scene.on_keyrelease(code)
end

function scene.on_text(text)
end

return scene
```

### Scene Lifecycle Methods

| Method | Required | Description |
|--------|----------|-------------|
| `on_enter()` | Yes | Called when scene becomes active. The `pool` table is populated before this is called. |
| `on_loop(delta)` | No | Called every frame with delta time in seconds |
| `on_tick(tick)` | No | Called at fixed intervals (if `with_ticks` configured) |
| `on_leave()` | No | Called when transitioning to another scene |
| `on_camera(delta)` | No* | Return camera viewport as Quad. *Required when layer type is `tilemap` |
| `on_touch(x, y)` | No | Called on mouse release when no object was hit. Receives click coordinates. |
| `on_motion(x, y)` | No | Called on mouse movement with cursor coordinates |
| `on_keypress(code)` | No | Called on key press |
| `on_keyrelease(code)` | No | Called on key release |
| `on_text(text)` | No | Called for text input |

### Scene Lifecycle Flow

```
scenemanager:register("scenename")
  └─> Load scenes/{name}.json
  └─> Create physics world with gravity
  └─> Load sound effects from "effects" array
  └─> Create layer (background or tilemap with colliders)
  └─> Create objects from "objects" array
  └─> Create particle emitters from "particles" array
  └─> Load scenes/{name}.lua
  └─> Wire up callbacks

scenemanager:set("scenename")
  └─> Previous scene on_leave()
  └─> For each object: on_dispose()
  └─> Stop all sounds
  └─> Populate pool table with objects, sounds, and particles
  └─> New scene on_enter()
  └─> For each object: on_spawn()

scenemanager:destroy("scenename")
  └─> Unload scene from memory
  └─> Use "*" to destroy all scenes except current
```

### Touch and Hover Behavior

When the user clicks/touches the screen:
1. The engine queries all objects at that position
2. If an object with `on_touch` is found, the object's callback is invoked
3. If no object is hit, the scene's `on_touch(x, y)` is called

When the mouse moves:
1. The engine tracks which objects are under the cursor
2. Objects that were hovered but are no longer receive `on_unhover`
3. Objects that are newly hovered receive `on_hover`
4. The scene's `on_motion(x, y)` is always called

---

## Object System

Objects are game entities with sprites, animations, and behavior. Each object type requires:
- A JSON file at `objects/<scene>/<kind>.json` defining animations and hitboxes
- A Lua file at `objects/<scene>/<kind>.lua` defining behavior
- A PNG sprite at `blobs/<scene>/<kind>.png`

### Object Lua File

Located at `objects/<scene>/<kind>.lua`:

```lua
return {
  on_spawn = function()
    self.health = 100
  end,

  on_dispose = function()
  end,

  on_loop = function(delta)
    if keyboard.left then
      self.x = self.x - 100 * delta
      self.flip = Flip.horizontal
    end
  end,

  on_touch = function(x, y)
  end,

  on_hover = function()
    self.action = "hover"
  end,

  on_unhover = function()
    self.action = "default"
  end,

  on_begin = function()
  end,

  on_end = function()
  end,

  on_collision = function(other_id, other_kind)
  end,

  on_collision_end = function(other_id, other_kind)
  end,

  on_tick = function(tick)
  end
}
```

### Object Lifecycle Methods

| Method | Description |
|--------|-------------|
| `on_spawn()` | Called when scene enters and object is created |
| `on_dispose()` | Called when scene leaves |
| `on_loop(delta)` | Called every frame |
| `on_touch(x, y)` | Called when this object is clicked/touched |
| `on_hover()` | Called when mouse enters object bounds (requires hitbox) |
| `on_unhover()` | Called when mouse leaves object bounds (requires hitbox) |
| `on_begin()` | Called when animation timeline starts |
| `on_end()` | Called when oneshot animation ends |
| `on_collision(id, kind)` | Called when physics sensor overlap begins |
| `on_collision_end(id, kind)` | Called when physics sensor overlap ends |
| `on_tick(tick)` | Called at fixed tick intervals |

### Object Properties

Inside object callbacks, `self` provides access to the entity:

| Property | Type | Access | Description |
|----------|------|--------|-------------|
| `id` | uint64 | read | Unique entity ID |
| `name` | string | read | Object instance name |
| `kind` | string | read/write | Object type |
| `x` | float | read/write | X position |
| `y` | float | read/write | Y position |
| `position` | Vec2 | read/write | Position as Vec2 |
| `z` | int | read/write | Render order (lower = back, changing triggers re-sort) |
| `alpha` | uint8 | read/write | Transparency (0-255) |
| `angle` | double | read/write | Rotation in degrees |
| `scale` | float | read/write | Scale multiplier |
| `flip` | Flip | read/write | `none`, `horizontal`, `vertical`, `both` |
| `visible` | bool | read/write | Render visibility |
| `action` | string | read/write | Current animation timeline (setting resets frame to 0) |
| `alive` | bool | read | Whether entity still exists |

### Object Methods

| Method | Description |
|--------|-------------|
| `self:die()` | Destroy the entity (also destroys physics body if present) |
| `self:clone()` | Create a copy of this object with a new unique name |

### Dynamic Callback Assignment

Callbacks can be reassigned at runtime via pool:

```lua
pool.button.on_hover = function()
  pool.button.action = "highlighted"
end

pool.button.on_touch = function(x, y)
  scenemanager:set("game")
end

pool.enemy.on_collision = function(other_id, other_kind)
  if other_kind == "bullet" then
    self:die()
  end
end

pool.player.on_tick = function(tick)
end
```

### Observable Properties

Objects support reactive properties through the key-value store. Custom properties can be observed for changes:

```lua
self:subscribe("health", function(value)
  if value <= 0 then
    self:die()
  end
end)

self.health = 100

self:unsubscribe("health")
```

Observable values support arithmetic operations and comparisons directly.

---

## Pool System

The `pool` global table is populated before `on_enter` is called and provides access to all scene resources: objects, sounds, and particles.

### Accessing Objects

Objects are accessed by their `name` defined in the scene JSON:

```lua
pool.player.x = 100
pool.player.y = 200
pool.player.action = "walk"
pool.player.visible = false
pool.player.alpha = 128
pool.player.angle = 45
pool.player.scale = 2.0
pool.player.z = 100
pool.player.flip = Flip.horizontal
```

See [Object System](#object-system) for the complete list of properties and methods.

### Cloning Objects

```lua
local clone = pool.player:clone()
clone.x = 200
clone.y = 300
```

The clone receives a unique name (e.g., `player_1`) and inherits all properties and scripts from the original.

### Custom Method Calls (Pool Proxy Pattern)

When you call a method on a pool object, it triggers the corresponding callback in the object's Lua file:

```lua
pool.enemy.take_damage(10)
```

This calls `on_take_damage` in the object's Lua file:

```lua
return {
  on_take_damage = function(amount)
    self.health = self.health - amount
    if self.health <= 0 then
      self:die()
    end
  end
}
```

### Accessing Sound Effects

Sound effects defined in the scene's `effects` array are available in pool:

```lua
pool.explosion:play(false)
pool.music:play(true)
pool.music:stop()
pool.music.volume = 0.5
```

### Accessing Particles

Particle emitters defined in the scene's `particles` array are available in pool:

```lua
pool.rain.spawning = true
pool.rain.spawning = false
pool.rain.position = {x = 100, y = 200}
```

---

## Particle System

Particles are controlled via pool:

```lua
pool.rain.spawning = true
pool.rain.spawning = false
pool.rain.position = {x = 100, y = 200}
```

### Dynamic Creation

```lua
local factory = particlesystem.factory
local batch = factory:create("spark", 100, 200, true)
```

---

## Tilemap System

Tilemaps are referenced in scene JSON via the layer property.

Collider layers create Box2D static bodies for non-zero tiles.

---

## Cursor Definition

### Usage

```lua
overlay:cursor("arrow")
overlay:dispatch("damage")
overlay:cursor(nil)
```

---

## Font Definition

Fonts are bitmap-based. Each font requires a JSON file and a corresponding PNG file.

### Font PNG Structure

The font PNG file must be located at `blobs/overlay/<fontname>.png`. The image uses a separator color to delimit glyphs:

1. The **first pixel** (top-left, position 0,0) defines the separator color
2. Glyphs are arranged horizontally, separated by columns of the separator color
3. The engine scans left-to-right, detecting glyph boundaries by the separator color
4. Each glyph's width is determined by the distance between separator columns
5. Each glyph's height is determined by scanning downward until the separator color is found

Example layout:
```
[S][A][A][A][S][B][B][S][C][C][C][C][S]...
[S][A][A][A][S][B][B][S][C][C][C][C][S]...
[S][A][A][A][S][B][B][S][C][C][C][C][S]...
```
Where `S` is the separator color and `A`, `B`, `C` are glyph pixels.

The glyphs in the PNG must match the order specified in the `glyphs` string in the JSON file.

### Usage

```lua
local label = overlay:label("rpgfont")
label:set("Hello!", x, y)
label.effect = {
  [1] = { alpha = 128, scale = 1.0, xoffset = 0, yoffset = 0, r = 255, g = 255, b = 255 }
}
label:clear()
overlay:label(label)
```

---

## Input Handling

### Keyboard (Polled)

The `keyboard` global table allows polling any key by name. Any valid key name can be used as a property:

```lua
if keyboard.space then end
if keyboard.left then end
if keyboard.right then end
if keyboard.up then end
if keyboard.down then end
if keyboard.a then end
if keyboard.b then end
if keyboard.escape then end
if keyboard.lshift then end
if keyboard.rctrl then end
```

### Keyboard Events

```lua
function scene.on_keypress(keycode)
  if keycode == KeyEvent.space then end
  if keycode == KeyEvent.enter then end
  if keycode == KeyEvent.backspace then end
end
```

### Mouse

The `mouse` global provides position and button state:

```lua
local x, y = mouse.x, mouse.y

local button = mouse.button
if button == MouseButton.left then end
if button == MouseButton.middle then end
if button == MouseButton.right then end
```

### Gamepad

```lua
local player = gamepads[Player.one]
if player.connected then
  if player:button(GamepadButton.a) then end
  if player:button(GamepadButton.up) then end

  local lx, ly = player:leftstick()
  local rx, ry = player:rightstick()
  local lt, rt = player:triggers()

  local x = player:axis(GamepadAxis.leftx)
  local name = player.name
end
```

| Property/Method | Description |
|-----------------|-------------|
| `gamepads[0..3]` | Access gamepad by slot (0-3) |
| `gamepads.count` | Number of connected gamepads |
| `connected` | Whether gamepad is connected |
| `name` | Gamepad name (or nil) |
| `button(btn)` | Check if button is pressed |
| `axis(ax)` | Get axis value (-32768 to 32767) |
| `leftstick()` | Returns x, y of left stick |
| `rightstick()` | Returns x, y of right stick |
| `triggers()` | Returns left, right trigger values |

| GamepadButton | Description |
|---------------|-------------|
| `GamepadButton.a` | A button (Xbox) / Cross (PS) |
| `GamepadButton.b` | B button (Xbox) / Circle (PS) |
| `GamepadButton.x` | X button (Xbox) / Square (PS) |
| `GamepadButton.y` | Y button (Xbox) / Triangle (PS) |
| `GamepadButton.up` | D-pad up |
| `GamepadButton.down` | D-pad down |
| `GamepadButton.left` | D-pad left |
| `GamepadButton.right` | D-pad right |
| `GamepadButton.start` | Start button |
| `GamepadButton.back` | Back/Select button |
| `GamepadButton.guide` | Guide/Home button |
| `GamepadButton.leftstick` | Left stick press |
| `GamepadButton.rightstick` | Right stick press |
| `GamepadButton.leftshoulder` | Left bumper (LB) |
| `GamepadButton.rightshoulder` | Right bumper (RB) |

| GamepadAxis | Description |
|-------------|-------------|
| `GamepadAxis.leftx` | Left stick X axis |
| `GamepadAxis.lefty` | Left stick Y axis |
| `GamepadAxis.rightx` | Right stick X axis |
| `GamepadAxis.righty` | Right stick Y axis |
| `GamepadAxis.triggerleft` | Left trigger |
| `GamepadAxis.triggerright` | Right trigger |

---

## Audio System

### Playing Sounds

```lua
pool.music:play()
pool.music:play(true)
pool.music:stop()
```

### Volume Control

```lua
pool.music.volume = 0.5
```

### Callbacks

```lua
pool.music.on_begin = function() end
pool.music.on_end = function() end
```

---

## Persistent Storage

The cassette system provides persistent key-value storage.

### Setting Values

```lua
cassette:set("highscore", 1000)
cassette:set("name", "Player")
cassette:set("completed", true)
```

### Getting Values

```lua
local score = cassette:get("highscore", 0)
local name = cassette:get("name", "Unknown")
```

### Clearing

```lua
cassette:clear("highscore")
cassette:clear()
```

---

## Localization

The engine provides a built-in localization system. Locale files are JSON files located in the `locales/` directory, named by language code (e.g., `en.json`, `pt.json`).

### Locale File Structure

Each locale file contains key-value pairs where keys are identifiers and values are translated strings:

```
locales/
├── en.json
└── pt.json
```

### Usage

The global function `_()` retrieves localized strings by key:

```lua
local text = _("greeting")
local start = _("menu.start")
local message = _("dialog.welcome")
```

The engine automatically selects the appropriate locale file based on system language settings.

---

## Canvas

The canvas provides direct pixel manipulation for custom rendering effects. It creates a RGBA32 framebuffer that is rendered on top of the scene every frame.

### Properties

| Property | Type | Description |
|----------|------|-------------|
| `pixels` | string | Raw RGBA32 pixel data (write-only) |

### Methods

| Method | Description |
|--------|-------------|
| `clear()` | Clear the framebuffer to transparent |

### Usage

```lua
canvas.pixels = pixel_data
canvas:clear()
```

The framebuffer size matches the viewport dimensions divided by the scale factor. The pixel data must be a string containing raw RGBA32 values (4 bytes per pixel: red, green, blue, alpha).

---

## Engine Globals

| Global | Description |
|--------|-------------|
| `engine` | Engine instance |
| `scenemanager` | Scene management |
| `gamepads` | Gamepad/controller state |
| `overlay` | UI layer (labels, cursor) |
| `canvas` | Pixel manipulation framebuffer |
| `viewport` | `{width, height}` |
| `pool` | Scene resources |
| `cassette` | Persistent storage |
| `keyboard` | Keyboard state (any key accessible by name) |
| `mouse` | Mouse state (`x`, `y`, `button`) |

### SceneManager

```lua
scenemanager:register("name")
scenemanager:set("name")
scenemanager:destroy("name")
scenemanager:destroy("*")
scenemanager.current
```

### Viewport

```lua
viewport.width
viewport.height
```

### Time

```lua
moment()
```

### Platform

```lua
operatingsystem:name()
desktop:folder()
```

### Query Parameters

```lua
queryparam("scene", "prelude")
```

### Achievements

```lua
achievement:unlock("ACH_ID")
```

### URL

```lua
openurl("https://example.com")
```

---

## JSON Schemas

### Scene Schema

Located at `scenes/<scene>.json`:

```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "type": "object",
  "properties": {
    "type": {
      "type": "string",
      "enum": ["backdrop"]
    },
    "width": {
      "type": "number",
      "minimum": 1
    },
    "height": {
      "type": "number",
      "minimum": 1
    },
    "physics": {
      "type": "object",
      "properties": {
        "gravity": {
          "type": "object",
          "properties": {
            "x": { "type": "number" },
            "y": { "type": "number" }
          },
          "required": ["x", "y"]
        }
      }
    },
    "layer": {
      "type": "object",
      "properties": {
        "type": {
          "type": "string",
          "enum": ["background", "tilemap"]
        },
        "content": {
          "type": "string"
        }
      },
      "required": ["type"]
    },
    "effects": {
      "type": "array",
      "items": { "type": "string" }
    },
    "objects": {
      "type": "array",
      "items": {
        "type": "object",
        "properties": {
          "name": { "type": "string" },
          "kind": { "type": "string" },
          "action": { "type": "string" },
          "x": { "type": "number" },
          "y": { "type": "number" }
        },
        "required": ["name", "kind"]
      }
    },
    "particles": {
      "type": "array",
      "items": {
        "type": "object",
        "properties": {
          "name": { "type": "string" },
          "kind": { "type": "string" },
          "x": { "type": "number" },
          "y": { "type": "number" },
          "spawning": { "type": "boolean" }
        },
        "required": ["name", "kind"]
      }
    }
  }
}
```

### Object Schema

Located at `objects/<scene>/<kind>.json`:

```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "type": "object",
  "properties": {
    "scale": {
      "type": "number",
      "minimum": 0
    },
    "timelines": {
      "type": "object",
      "additionalProperties": {
        "type": "object",
        "properties": {
          "oneshot": { "type": "boolean" },
          "next": { "type": ["string", "null"] },
          "hitbox": {
            "type": "object",
            "properties": {
              "aabb": {
                "type": "object",
                "properties": {
                  "x": { "type": "number" },
                  "y": { "type": "number" },
                  "w": { "type": "number", "minimum": 0 },
                  "h": { "type": "number", "minimum": 0 }
                },
                "required": ["x", "y", "w", "h"]
              }
            }
          },
          "frames": {
            "type": "array",
            "items": {
              "type": "object",
              "properties": {
                "duration": { "type": "number" },
                "offset": {
                  "type": "object",
                  "properties": {
                    "x": { "type": "number" },
                    "y": { "type": "number" }
                  },
                  "required": ["x", "y"]
                },
                "quad": {
                  "type": "object",
                  "properties": {
                    "x": { "type": "number" },
                    "y": { "type": "number" },
                    "w": { "type": "number", "minimum": 0 },
                    "h": { "type": "number", "minimum": 0 }
                  },
                  "required": ["x", "y", "w", "h"]
                }
              },
              "required": ["duration", "offset", "quad"]
            }
          }
        },
        "required": ["frames"]
      }
    }
  },
  "required": ["timelines"]
}
```

### Particle Schema

Located at `particles/<kind>.json`:

```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "type": "object",
  "definitions": {
    "range": {
      "type": "object",
      "properties": {
        "start": { "type": "number" },
        "end": { "type": "number" }
      },
      "required": ["start", "end"]
    }
  },
  "properties": {
    "count": {
      "type": "integer",
      "minimum": 1
    },
    "spawn": {
      "type": "object",
      "properties": {
        "x": { "$ref": "#/definitions/range" },
        "y": { "$ref": "#/definitions/range" },
        "radius": { "$ref": "#/definitions/range" },
        "angle": { "$ref": "#/definitions/range" },
        "scale": { "$ref": "#/definitions/range" },
        "life": { "$ref": "#/definitions/range" },
        "alpha": { "$ref": "#/definitions/range" }
      }
    },
    "velocity": {
      "type": "object",
      "properties": {
        "x": { "$ref": "#/definitions/range" },
        "y": { "$ref": "#/definitions/range" }
      }
    },
    "gravity": {
      "type": "object",
      "properties": {
        "x": { "$ref": "#/definitions/range" },
        "y": { "$ref": "#/definitions/range" }
      }
    },
    "rotation": {
      "type": "object",
      "properties": {
        "force": { "$ref": "#/definitions/range" },
        "velocity": { "$ref": "#/definitions/range" }
      }
    }
  },
  "required": ["count"]
}
```

### Tilemap Schema

Located at `tilemaps/<name>.json`:

```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "type": "object",
  "properties": {
    "tile_size": {
      "type": "integer",
      "minimum": 1
    },
    "width": {
      "type": "integer",
      "minimum": 1
    },
    "height": {
      "type": "integer",
      "minimum": 1
    },
    "layers": {
      "type": "array",
      "items": {
        "type": "object",
        "properties": {
          "name": { "type": "string" },
          "collider": { "type": "boolean" },
          "tiles": {
            "type": "array",
            "items": { "type": "integer", "minimum": 0 }
          }
        },
        "required": ["tiles"]
      }
    }
  },
  "required": ["tile_size", "width", "height", "layers"]
}
```

### Cursor Schema

Located at `cursors/<name>.json`:

```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "type": "object",
  "properties": {
    "point": {
      "type": "object",
      "properties": {
        "x": { "type": "number" },
        "y": { "type": "number" }
      },
      "required": ["x", "y"]
    },
    "animations": {
      "type": "object",
      "additionalProperties": {
        "type": "object",
        "properties": {
          "oneshot": { "type": "boolean" },
          "frames": {
            "type": "array",
            "items": {
              "type": "object",
              "properties": {
                "duration": { "type": "number" },
                "offset": {
                  "type": "object",
                  "properties": {
                    "x": { "type": "number" },
                    "y": { "type": "number" }
                  },
                  "required": ["x", "y"]
                },
                "quad": {
                  "type": "object",
                  "properties": {
                    "x": { "type": "number" },
                    "y": { "type": "number" },
                    "w": { "type": "number", "minimum": 0 },
                    "h": { "type": "number", "minimum": 0 }
                  },
                  "required": ["x", "y", "w", "h"]
                }
              },
              "required": ["duration", "offset", "quad"]
            }
          }
        },
        "required": ["frames"]
      }
    }
  },
  "required": ["point", "animations"]
}
```

### Font Schema

Located at `fonts/<name>.json`:

```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "type": "object",
  "properties": {
    "glyphs": {
      "type": "string",
      "minLength": 1
    },
    "spacing": {
      "type": "integer",
      "description": "Horizontal spacing between glyphs in pixels"
    },
    "leading": {
      "type": "integer",
      "description": "Vertical spacing between lines in pixels"
    },
    "scale": {
      "type": "number",
      "minimum": 0,
      "description": "Scale multiplier for glyph rendering"
    }
  },
  "required": ["glyphs", "spacing", "leading"]
}
```

### Locale Schema

Located at `locales/<lang>.json`:

```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "type": "object",
  "additionalProperties": {
    "type": "string"
  }
}
```

---

## Types

| Type | Constructor | Description |
|------|-------------|-------------|
| Vec2 | `Vec2.new()` or `Vec2.new(x, y)` | 2D vector with `x`, `y` fields |
| Vec3 | `Vec3.new()` or `Vec3.new(x, y, z)` | 3D vector with `x`, `y`, `z` fields |
| Quad | `Quad.new()` or `Quad.new(x, y, w, h)` | Rectangle with `x`, `y`, `width`, `height` fields |
| Color | `Color.new("#ff0000")` | Color from hex string with `r`, `g`, `b`, `a` properties |

---

## Enums

### Flip

| Value | Description |
|-------|-------------|
| `Flip.none` | No flip |
| `Flip.horizontal` | Horizontal flip |
| `Flip.vertical` | Vertical flip |
| `Flip.both` | Both axes flipped |

### KeyEvent

| Value | Description |
|-------|-------------|
| `KeyEvent.up` | Up arrow |
| `KeyEvent.down` | Down arrow |
| `KeyEvent.left` | Left arrow |
| `KeyEvent.right` | Right arrow |
| `KeyEvent.space` | Space bar |
| `KeyEvent.enter` | Enter key |
| `KeyEvent.backspace` | Backspace key |

### MouseButton

| Value | Description |
|-------|-------------|
| `MouseButton.none` | No button |
| `MouseButton.left` | Left button |
| `MouseButton.middle` | Middle button |
| `MouseButton.right` | Right button |
