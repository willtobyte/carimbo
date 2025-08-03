# Carimbo Lua API Documentation

## Platforms

Carimbo runs on multiple platforms—Windows, Linux, macOS, Android, iOS, WebAssembly, etc.—but currently, releases are only compiled for Windows (amd64), macOS (Apple Silicon), and WebAssembly.

## Lua

Carimbo supports both PUC-Rio Lua and LuaJIT.
PUC-Rio Lua is only used when compiling to WebAssembly or targeting iOS, since LuaJIT is not compatible with WebAssembly and JIT compilation is not permitted on iOS. For all other platforms, LuaJIT is used to take advantage of its Just-In-Time compilation performance benefits.

**It’s important to always write code that is compatible with both runtimes** to ensure maximum portability across all supported platforms.

## Steam

Carimbo allows unlocking achievements through a simple API, without requiring dynamic linking to Valve’s DLL.
Here’s an example:

```lua
achievement:unlock("achievement_id")
```

It works on macOS (Apple Silicon) and Windows (amd64), and it was also tested and confirmed to work on Proton for Linux without any necessary changes.

## The essential

### Game Filesystem Structure

Any game made with the Carimbo must follow the following structure.

```
.
├── blobs
│   ├── myscene
│   │   ├── background.png
│   │   └── myitem.png
│   └── overlay
│       ├── myfont.png
│       └── mycursor.png
├── cursors
│   └── mycursor.json
├── fonts
│   └── myfont.json
├── objects
│   └── myscene
│       └── myitem.json
├── scenes
│   ├── myscene.json
│   └── myscene.lua
└── scripts
    ├── helpers
    │   └── functional.lua
    └── main.lua
```

`blobs/`

Binary asset storage for all media used by the game.

*Subdirectories:*
* myscene/: Assets specific to a given scene.
* overlay/: UI-related assets, both visual and audio.

*Includes:*
* Images (`.png`): Scene backgrounds or objects.
* Audio (`.ogg`): Sound effects or music.

`cursors/`

Cursor definitions in `.json` format.

`fonts/`

Font metadata files in `.json` format used by the UI renderer.

`objects/`

Scene-specific object definitions in `.json` format, grouped by scene slug.

`scenes/`

Scene logic and configuration. Must contains:
* A `.json` metadata file.
* A `.lua` script for behavior.

`scripts/`

All general Lua code.
* `helpers/`: Shared helper modules.
* `main.lua`: Game entry point.


### Engine Initialization

It must be located inside `scripts/main.lua`.

```lua
_G.engine = EngineFactory.new()
  :with_title("Untitled")
  :with_width(1920)
  :with_height(1080)
  :with_scale(4.0)
  :with_fullscreen(true)
  :create()
```

This block creates a new engine instance using a fluent interface provided by _EngineFactory_. Here’s what each method does:
* `with_title("Untitled")`: Sets the window title to "Untitled".
* `with_width(1920)`: Sets the window width to 1920 pixels.
* `with_height(1080)`: Sets the window height to 1080 pixels.
* `with_scale(4.0)`: Applies a render scaling factor of 4.0 (useful for retro pixel art aesthetics).
* `with_fullscreen(true)`: Launches the engine in fullscreen mode.
* `create()`: Finalizes the setup and returns the initialized engine object.

The engine is stored globally as `_G.engine`.

#### Lifecycle Hooks

```lua
function setup()
end

function loop()
end
```

* `setup()`: Called once when the engine starts. Use this function to initialize game objects, load resources, or prepare state.
* `loop()`: Called every frame. This is the main game loop where game logic, input handling, and rendering should be performed.

Both functions are empty placeholders here and should be filled with game-specific logic.

They must **mandatorily** be declared in `scripts/main.lua`, even if left empty.

### Managers & Miscellaneous

The **engine** instance, now made global for convenience, holds the following components:
* `canvas` an object that has a pixels property, where you can write arbitrary pixels to the screen — think of it like a poor man’s shader.
* `cassette` remember when old computers and some consoles used cassette tapes to store programs? Same logic — you can use it as a key-value store to save game data.
* `objectmanager` manager responsible for creating and destroying objects
* `fontfactory` manager responsible for loading bitmap fonts.
* `overlay` manager responsible for handling the Heads-Up Display (HUD)
* `resourcemanager` manager responsible for batch loading assets and flushing unused ones.
* `soundmanager` manager responsible for playing and stopping sounds, as well as flushing them.
* `statemanager` manager responsible for allowing instant querying of input state.
* `scenemanager` manager responsible for registering and setting the current scenario.
* `timermanager` manager responsible for creating periodic and single-shot timers.

Example:

```lua
local timermanager = engine:timermanager()

local id

function scene.on_enter()
  local delay = 600
  id = timermanager:set(delay, function()
    print("On timer.")
  end)
end

function scene.on_leave()
  timermanager:clear(id)
end
```

### Canvas

```lua
-- Effect module that fills the entire canvas with a solid red color.

local Effect = {}
Effect.__index = Effect

-- Local aliases for performance
local rep, char = string.rep, string.char

function Effect:new()
  local width, height = 480, 270

  -- Define a single red pixel (BGRA = 0, 0, 255, 255)
  local pixel = char(0, 0, 255, 255)

  -- One horizontal line of red pixels
  local line = rep(pixel, width)

  -- Full frame precomputed: vertical repetition of red lines
  local frame = rep(line, height)

  return setmetatable({
      canvas = engine:canvas(),
      frame = frame,
    }, self)
end

-- Loop function: called every frame to render the effect
function Effect:loop()
  -- Set the entire canvas to the precomputed red frame
  self.canvas.pixels = self.frame
end

-- Return an initialized instance of Effect
return Effect:new()
```

How to use:

```lua
local effect = require("effects/effect")

local scene = {}

function scene.on_loop()
	effect:loop()
end

return scene
```

### Cassette

### Scenes

Every game needs at least one scene. It is used to load objects, sound effects, and handle input logic through callbacks.

```lua
-- Scene script with basic setup, interaction, and cleanup logic.

local scene = {}

-- Object pool.
local pool = {}

function scene.on_enter()
  -- On scene enter.

  -- Retrieve a sound effect from the scenario.
  pool.theme = scene:get("theme", SceneType.effect)
  pool.theme:play(true)

  -- Retrieve an object from the scenario, useful when you need to manipulate it.
  pool.object = scene:get("object", SceneType.object)
end

function scene.on_loop()
  -- Every loop.
end

function scene.on_text(text)
  -- On text input.
end

function scene.on_keypress(code)
  -- On key press.

  -- Set an action for the play of the object.
  pool.object.action = "play"
end

function scene.on_motion(x, y)
  -- On mouse motion.
end

function scene.on_leave()
  -- On scene leave.

  -- No need to stop any sound effects manually.
  -- SceneManager will handle stopping all sounds before the transition.

  -- Clean up the object pool.
  for o in pairs(pool) do
    pool[o] = nil
  end
end

return scene
```

Before using it, you must always `register` the scene. This can be done all at once during the engine’s boot process, or lazily as the player progresses, like this:

After registering one or more scenes, call `set` to make the SceneManager use the selected scene.

```lua
local scenemanager = engine:scenemanager()

function setup()
  scenemanager:register("myscene")
  -- ...Other scenes.

  scenemanager:set("myscene")
end
```
