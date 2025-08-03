

### File structure

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
