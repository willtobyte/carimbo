#include "scriptengine.hpp"

inline constexpr auto bootstrap =
#include "bootstrap.lua"
;

inline constexpr auto debugger =
#ifdef EMSCRIPTEN
  R"lua(
    local noop = function() end

    _G.dbg = setmetatable({}, {
      __index = function(table, key)
        return noop
      end,
      __call = noop
    })
  )lua";
#else
#include "debugger.lua"
#endif
;

static int on_panic(lua_State* L) {
  const auto* message = lua_tostring(L, -1);
  throw std::runtime_error(std::format("Lua panic: {}", message));
}

static sol::object searcher(sol::this_state state, std::string_view module) {
  sol::state_view lua{state};

  const auto filename = std::format("scripts/{}.lua", module);
  const auto buffer = io::read(filename);
  std::string_view script{reinterpret_cast<const char*>(buffer.data()), buffer.size()};

  const auto loader = lua.load(script, std::format("@{}", filename));
  verify(loader);
  return sol::make_object(lua, loader.get<sol::protected_function>());
}

class lua_loopable final : public loopable {
public:
  explicit lua_loopable(const sol::state_view lua)
      : _L(lua), _start(SDL_GetTicks()) {}

  void loop(float delta) override {
    _frames++;
    const auto now = SDL_GetTicks();
    _elapsed += now - _start;
    _start = now;

    const auto memory = lua_gc(_L, LUA_GCCOUNT, 0);

    if (_elapsed >= 1000) [[unlikely]] {
      std::println("{:.1f} {}KB", static_cast<double>(_frames) * 1000.0 / static_cast<double>(_elapsed), memory);

      _elapsed = 0;
      _frames = 0;
    }

    if (memory <= 8192) [[likely]] {
      lua_gc(_L, LUA_GCSTEP, 8);
      return;
    }

    lua_gc(_L, LUA_GCCOLLECT, 0);
    lua_gc(_L, LUA_GCCOLLECT, 0);
  }

private:
  lua_State* _L;
  uint64_t _frames{0};
  uint64_t _elapsed{0};
  uint64_t _start;
};

struct sentinel final {
  std::string _name;

  explicit sentinel(std::string name) noexcept
      : _name(std::move(name)) {}

  ~sentinel() noexcept {
    std::println("[garbagecollector] collected {}", _name);
  }
};

void scriptengine::run() {
  const auto start = SDL_GetPerformanceCounter();

  sol::state lua;
  lua.open_libraries();
  lua.set_panic(&on_panic);

  lua["searcher"] = &searcher;

  constexpr auto inject = R"lua(
    local list = package.searchers or package.loaders
    table.insert(list, searcher)
  )lua";

  lua.script(inject);

  lua["sentinel"] = [&lua](sol::object object, sol::object name) {
    auto instance = sol::make_object<sentinel>(lua, name.as<std::string>());
    object.as<sol::table>().raw_set("__sentinel", instance);

    return instance;
  };

  lua["math"]["random"] = sol::overload(
    []() noexcept { return rng::engine().uniform(); },
    [](lua_Integer upper) noexcept { return rng::engine().range<lua_Integer>(1, upper); },
    [](lua_Integer low, lua_Integer high) noexcept { return rng::engine().range<lua_Integer>(low, high); }
  );

  lua["math"]["randomseed"] = [](lua_Integer seed) noexcept { rng::seed(static_cast<uint64_t>(seed)); };

  lua["_"] = &localization::text;

  lua["moment"] = []() noexcept { return SDL_GetTicks(); };

  lua["openurl"] = [](std::string_view url) {
#ifdef EMSCRIPTEN
    const auto script = std::format(R"javascript(window.open('{}', '_blank', 'noopener,noreferrer');)javascript", url);
    emscripten_run_script(script.c_str());
#else
    SDL_OpenURL(url.data());
#endif
  };

  lua["queryparam"] = [](std::string_view key, std::string_view defval) {
    std::string out{defval};

#ifdef EMSCRIPTEN
    const auto script = std::format(
        R"javascript(
          new URLSearchParams(location.search).get("{}") ?? "{}"
        )javascript",
        key,
        defval
    );

    if (const auto* result = emscripten_run_script_string(script.c_str()); result && *result) {
      out.assign(result);
    }
#else
    std::array<char, 64> uppercase_key{};
    const auto len = std::min(key.size(), uppercase_key.size() - 1);
    for (std::size_t i = 0; i < len; ++i) {
      uppercase_key[i] = static_cast<char>(std::toupper(static_cast<unsigned char>(key[i])));
    }

    if (const auto* value = std::getenv(uppercase_key.data()); value && *value) {
      out.assign(value);
    }
#endif

    return out;
  };

  steam::achievement achievement;

  lua.new_usertype<steam::achievement>(
    "Achievement",
    "unlock", &steam::achievement::unlock
  );

  lua["achievement"] = &achievement;

  desktop desktop;

  lua.new_usertype<::desktop>(
    "Desktop",
    "folder", &desktop::folder
  );

  lua["desktop"] = &desktop;

  operatingsystem operatingsystem;

  lua.new_usertype<::operatingsystem>(
    "OperatingSystem",
    "compute", &operatingsystem::compute,
    "memory", &operatingsystem::memory,
    "name", &operatingsystem::name
  );

  lua["operatingsystem"] = &operatingsystem;

  lua.new_usertype<soundfx>(
    "SoundFX",
    sol::no_constructor,
     "play", [](soundfx& self, std::optional<bool> loop_opt) {
       const auto loop = loop_opt.value_or(false);
       self.play(loop);
     },
     "stop", &soundfx::stop,
     "volume", sol::property(&soundfx::volume, &soundfx::set_volume),
     "on_begin", &soundfx::set_onbegin,
     "on_end", &soundfx::set_onend
  );

  lua.new_usertype<soundmanager>(
    "SoundManager",
    sol::no_constructor
  );

  lua.new_enum(
    "SceneKind",
    "object", scenekind::object,
    "effect", scenekind::effect,
    "particle", scenekind::particle
  );

  lua.new_enum(
    "Controller",
    "up", event::gamepad::button::up,
    "down", event::gamepad::button::down,
    "left", event::gamepad::button::left,
    "right", event::gamepad::button::right,
    "north", event::gamepad::button::north,
    "east", event::gamepad::button::east,
    "south", event::gamepad::button::south,
    "west", event::gamepad::button::west
  );

  lua.new_enum(
    "Flip",
    "none", flip::none,
    "horizontal", flip::horizontal,
    "vertical", flip::vertical,
    "both", flip::both
  );

  lua.new_usertype<observable>(
    "Observable",
    sol::no_constructor,
    "value", sol::property(&observable::value),
    "set", &observable::set,
    "subscribe", &observable::subscribe,
    "unsubscribe", &observable::unsubscribe
  );

  lua.new_usertype<resourcemanager>(
    "ResourceManager",
    sol::no_constructor,
    "flush", [&lua](resourcemanager& self) {
      lua.collect_garbage();
      self.flush();
    },
    "prefetch", sol::overload(
        [](resourcemanager& self) {
          self.prefetch();
        },

        [](resourcemanager& self, sol::table table) {
          std::vector<std::string> filenames;
          filenames.reserve(table.size());
          for (const auto& [key, value] : table) {
            filenames.emplace_back(value.as<std::string>());
          }
          self.prefetch(std::move(filenames));
        },

        [](resourcemanager& self, sol::variadic_args arguments) {
          std::vector<std::string> filenames;
          filenames.reserve(arguments.size());
          for (const auto& value : arguments) {
            filenames.emplace_back(value.as<std::string>());
          }
          self.prefetch(std::move(filenames));
        }
      )
  );

  struct playerwrapper {
    uint8_t index;
    const statemanager* e;

    bool on(event::gamepad::button type) noexcept {
      return e->on(index, type);
    }
  };

  lua.new_usertype<playerwrapper>(
    "PlayerWrapper",
    sol::no_constructor,
    "on", &playerwrapper::on
  );

  lua.new_enum(
    "Player",
    "one", event::player::one,
    "two", event::player::two,
    "three", event::player::three,
    "four", event::player::four
  );

  lua.new_usertype<statemanager>(
    "StateManager",
    sol::no_constructor,
    "players", sol::property(&statemanager::players),
    "player", [](statemanager& self, event::player player) {
      static std::array<playerwrapper, 4> _p{
        playerwrapper{0, &self},
        playerwrapper{1, &self},
        playerwrapper{2, &self},
        playerwrapper{3, &self}
      };

      assert(static_cast<uint8_t>(player) < _p.size() && "invalid player index");
      return _p[static_cast<uint8_t>(player)];
    }
  );

  struct metaentity {
    static sol::object index(entityproxy& self, sol::stack_object key, sol::this_state state) {
      const auto ptr = self.kv.get(key.as<std::string_view>());
      return sol::make_object(state, std::ref(*ptr));
    }

    static void new_index(entityproxy& self, sol::stack_object key, sol::stack_object value) {
      self.kv.set(key.as<std::string_view>(), value);
    }
  };

  lua.new_usertype<entityproxy>(
    "Entity",
    sol::no_constructor,
    "id", sol::property(&entityproxy::id),
    "x", sol::property(&entityproxy::x, &entityproxy::set_x),
    "y", sol::property(&entityproxy::y, &entityproxy::set_y),
    "alpha", sol::property(&entityproxy::alpha, &entityproxy::set_alpha),
    "angle", sol::property(&entityproxy::angle, &entityproxy::set_angle),
    "scale", sol::property(&entityproxy::scale, &entityproxy::set_scale),
    "flip", sol::property(&entityproxy::flip, &entityproxy::set_flip),
    "visible", sol::property(&entityproxy::visible, &entityproxy::set_visible),
    "action", sol::property(
      [](const entityproxy& self) { return action_name(self.action()); },
      [](entityproxy& self, std::optional<std::string_view> name) {
        self.set_action(name ? _resolve(*name) : no_action);
      }
    ),
    "kind", sol::property(
      [](const entityproxy& self) { return action_name(self.kind()); },
      [](entityproxy& self, std::string_view name) {
        self.set_kind(_resolve(name));
      }
    ),
    "position", sol::property(
      &entityproxy::position,
      [](entityproxy& self, sol::table table) {
        const auto x = table.get_or("x", table.get_or(1, .0f));
        const auto y = table.get_or("y", table.get_or(2, .0f));
        self.set_position({x, y});
      }
    ),
    "on_mail", &entityproxy::set_onmail,
    "on_hover", &entityproxy::set_onhover,
    "on_unhover", &entityproxy::set_onunhover,
    "on_touch", &entityproxy::set_ontouch,
    "on_begin", &entityproxy::set_onbegin,
    "on_end", &entityproxy::set_onend,
    "clone", &entityproxy::clone,

    sol::meta_function::index, metaentity::index,
    sol::meta_function::new_index, metaentity::new_index
  );

  lua.new_usertype<scene>(
    "Scene",
    sol::no_constructor,
    "name", sol::property(&scene::name),
    "get", &scene::get
  );

  lua.new_usertype<scenemanager>(
    "SceneManager",
    sol::no_constructor,
    "current", sol::property(&scenemanager::current),
    "set", [&lua](scenemanager& self, std::string_view name) {
      try {
        self.set(name);
      } catch (const std::exception& e) {
        luaL_error(lua, "%s", e.what());
      }
    },
    "destroy", [&lua](
      scenemanager& self,
      std::string_view name
    ) {
      const auto scenes = self.query(name);

      auto loaded = lua["package"]["loaded"];
      for (const auto& scene : scenes) {
        loaded[std::format("scenes/{}", scene)] = sol::lua_nil;
      }

      lua.collect_garbage();
      lua.collect_garbage();

      self.destroy(name);

      lua.collect_garbage();
      lua.collect_garbage();
    },
    "register", [&lua](
      scenemanager& self,
      std::string_view name
    ) {
      const auto start = SDL_GetPerformanceCounter();

      try {
        const auto scene = self.load(name);
        if (!scene) [[unlikely]] {
          return;
        }

        const auto filename = std::format("scenes/{}.lua", name);
        const auto buffer = io::read(filename);
        std::string_view script{reinterpret_cast<const char*>(buffer.data()), buffer.size()};
        const auto result = lua.load(script, std::format("@{}", filename));
        verify(result);
        const auto pf = result.get<sol::protected_function>();
        const auto exec = pf();
        verify(exec);
        auto module = exec.get<sol::table>();

        auto loaded = lua["package"]["loaded"];
        loaded[std::format("scenes/{}", name)] = module;
        auto ptr = std::weak_ptr<::scene>(scene);

        module["get"] = [ptr, name](sol::table, std::string_view id, scenekind kind) {
          auto scene = ptr.lock();
          assert(scene && "scene should be valid");
          return scene->get(id, kind);
        };

        if (auto fn = module["on_enter"].get<sol::protected_function>(); fn.valid()) {
          const auto wrapper = [fn, ptr, &lua, module]() {
            lua["pool"] = lua.create_table();

            auto scene = ptr.lock();
            assert(scene && "scene should be valid");
            lua["timermanager"] = scene->timermanager();

            const auto result = fn();
            if (!result.valid()) {
              sol::error err = result;
              throw std::runtime_error(err.what());
            }

            if (auto onloop = module["on_loop"].get<sol::protected_function>(); onloop.valid()) {
              scene->set_onloop(std::move(onloop));
            }

            if (auto onmotion = module["on_motion"].get<sol::protected_function>(); onmotion.valid()) {
              scene->set_onmotion(std::move(onmotion));
            }

            if (auto oncamera = module["on_camera"].get<sol::protected_function>(); oncamera.valid()) {
              scene->set_oncamera(std::move(oncamera));
            }

            if (auto ontext = module["on_text"].get<sol::protected_function>(); ontext.valid()) {
              scene->set_ontext(std::move(ontext));
            }

            if (auto ontouch = module["on_touch"].get<sol::protected_function>(); ontouch.valid()) {
              scene->set_ontouch(std::move(ontouch));
            }

            if (auto onkeypress = module["on_keypress"].get<sol::protected_function>(); onkeypress.valid()) {
              scene->set_onkeypress(std::move(onkeypress));
            }

            if (auto onkeyrelease = module["on_keyrelease"].get<sol::protected_function>(); onkeyrelease.valid()) {
              scene->set_onkeyrelease(std::move(onkeyrelease));
            }

            if (auto onleave = module["on_leave"].get<sol::protected_function>(); onleave.valid()) {
              const auto wrapper = [onleave, &lua]() {
                const auto result = onleave();
                if (!result.valid()) {
                  sol::error err = result;
                  throw std::runtime_error(err.what());
                }

                lua.collect_garbage();
                lua.collect_garbage();

                lua["timermanager"] = sol::lua_nil;

                lua["pool"] = sol::lua_nil;

                lua.collect_garbage();
                lua.collect_garbage();
              };

              scene->set_onleave(std::move(wrapper));
            }
          };

          scene->set_onenter(std::move(wrapper));

          lua.collect_garbage();
          lua.collect_garbage();
        }

        const auto end = SDL_GetPerformanceCounter();
        const auto elapsed = static_cast<double>(end - start) * 1000.0 / static_cast<double>(SDL_GetPerformanceFrequency());
        std::println("[scriptengine] {} took {:.3f}ms", name, elapsed);
      } catch (const std::exception& e) {
        luaL_error(lua, e.what());
      }
    });

  lua.new_enum(
    "WidgetType",
    "cursor", ::widgettype::cursor,
    "label", ::widgettype::label
  );

  struct cursorproxy {
    overlay& o;

    void set(std::string_view name) noexcept { o.set_cursor(name); }

    void hide() noexcept { o.hide(); }
  };

  lua.new_usertype<cursorproxy>(
    "CursorProxy",
    sol::no_constructor,
    "set", &cursorproxy::set,
    "hide", &cursorproxy::hide
  );

  lua.new_usertype<overlay>(
    "Overlay",
    sol::no_constructor,
    "create", &overlay::create,
    "destroy", &overlay::destroy,
    "dispatch", &overlay::dispatch,
    "cursor", sol::property([](overlay& o) { return cursorproxy{o}; })
  );

  lua.new_usertype<particleprops>(
    "ParticleProps",
    sol::no_constructor,
    "active", sol::property(&particleprops::active),
    "spawning", sol::property(&particleprops::spawning),
    "position", sol::property(
      []() {
        return nullptr;
      },
      [](particleprops& self, sol::table table) {
        const auto x = table.get_or("x", table.get_or(1, .0f));
        const auto y = table.get_or("y", table.get_or(2, .0f));

        self.set_position(x, y);
      }
    )
  );

  lua.new_usertype<particlefactory>(
    "ParticleFactory",
    sol::no_constructor,
    "create", &particlefactory::create
  );

  lua.new_usertype<particlesystem>(
    "ParticleSystem",
    sol::no_constructor,
    "factory", sol::property(&particlesystem::factory),
    "add", &particlesystem::add,
    "clear", &particlesystem::clear,
    "set", &particlesystem::set
  );

  lua["cassette"] = cassette();

  lua.new_usertype<font>(
    "Font",
    sol::no_constructor,
    "glyphs", sol::property(&font::glyphs)
  );

  lua.new_usertype<enginefactory>(
    "EngineFactory",
    sol::constructors<enginefactory()>(),
    "with_title", &enginefactory::with_title,
    "with_width", &enginefactory::with_width,
    "with_height", &enginefactory::with_height,
    "with_scale", &enginefactory::with_scale,
    "with_gravity", &enginefactory::with_gravity,
    "with_fullscreen", &enginefactory::with_fullscreen,
    "with_sentry", &enginefactory::with_sentry,
    "create", &enginefactory::create
  );

  lua.new_usertype<vec2>(
    "Vec2",
    sol::constructors<vec2(), vec2(float, float)>(),
    "x", &vec2::x,
    "y", &vec2::y
  );

  lua.new_usertype<vec3>(
    "Vec3",
    sol::constructors<vec3(), vec3(float, float, float)>(),
    "x", &vec3::x,
    "y", &vec3::y,
    "z", &vec3::z
  );

  lua.new_usertype<quad>(
    "quad",
    sol::constructors<quad(), quad(float, float, float, float)>(),
    "x", &quad::x,
    "y", &quad::y,
    "width", &quad::w,
    "height", &quad::h
  );

  lua.new_usertype<cassette>(
    "Cassette",
    sol::no_constructor,
    "clear", &cassette::clear,
    "set", [](
      cassette& self,
      std::string_view key,
      sol::object value
    ) {
      switch (value.get_type()) {
        case sol::type::number: {
          if (value.is<int64_t>()) {
            self.set(key, value.as<int64_t>());
            break;
          }

          if (value.is<uint64_t>()) {
            self.set(key, value.as<uint64_t>());
            break;
          }

          self.set(key, value.as<double>());
          break;
        }
        case sol::type::boolean:
          self.set(key, value.as<bool>());
          break;
        case sol::type::string:
          self.set(key, value.as<std::string_view>());
          break;
        default:
          self.set(key, nullptr);
          break;
      }
    },
    "get", [](
      const cassette& self,
      std::string_view key,
      sol::object fallback_value,
      sol::this_state state
    ) {
      sol::state_view lua(state);

      switch (fallback_value.get_type()) {
        case sol::type::boolean: {
          const auto v = self.get<bool>(key, fallback_value.as<bool>());
          return sol::make_object(lua, v);
        }

        case sol::type::number: {
          const double x = fallback_value.as<double>();
          double i{};
          const double frac = std::modf(x, &i);

          if (std::fabs(frac) < std::numeric_limits<double>::epsilon()) {
            if (x < 0) {
              const auto v = self.get<int64_t>(key, static_cast<int64_t>(i));
              return sol::make_object(lua, v);
            }
            const auto v = self.get<uint64_t>(key, static_cast<uint64_t>(i));
            return sol::make_object(lua, v);
          }

          const auto v = self.get<double>(key, x);
          return sol::make_object(lua, v);
        }

        case sol::type::string: {
          const auto v = self.get<std::string>(key, fallback_value.as<std::string>());
          return sol::make_object(lua, v);
        }

        default: {
          const auto* value = self.find(key);
          if (!value) {
            return sol::make_object(lua, sol::lua_nil);
          }

          return std::visit([&lua](const auto& v) {
            using T = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<T, std::nullptr_t>) {
              return sol::make_object(lua, sol::lua_nil);
            } else {
              return sol::make_object(lua, v);
            }
          }, *value);
        }
      }
    }
  );

  lua.new_usertype<color>(
    "Color",
    "color", sol::constructors<color(std::string_view)>(),
    "r", sol::property(&color::r),
    "g", sol::property(&color::g),
    "b", sol::property(&color::b),
    "a", sol::property(&color::a),
    sol::meta_function::equal_to, &color::operator==
  );

  lua.new_enum(
    "KeyEvent",
    "up", event::keyboard::key::up,
    "left", event::keyboard::key::left,
    "down", event::keyboard::key::down,
    "right", event::keyboard::key::right,
    "space", event::keyboard::key::space,

    "backspace", event::keyboard::key::backspace,
    "enter", event::keyboard::key::enter,
    "escape", event::keyboard::key::escape
  );

  struct mouse final {
  private:
    [[nodiscard]] static std::tuple<float, float, uint32_t> state() noexcept {
      float x, y;
      const auto b = SDL_GetMouseState(&x, &y);
      return {x, y, b};
    }

  public:
    static float x() noexcept {
      const auto [x, y, b] = state();
      return x;
    }

    static float y() noexcept {
      const auto [x, y, b] = state();
      return y;
    }

    static std::tuple<float, float> xy() noexcept {
      const auto [x, y, b] = state();
      return {x, y};
    }

    static int button() noexcept {
      const auto [x, y, b] = state();
      if (b & SDL_BUTTON_MASK(SDL_BUTTON_LEFT)) return SDL_BUTTON_LEFT;
      if (b & SDL_BUTTON_MASK(SDL_BUTTON_MIDDLE)) return SDL_BUTTON_MIDDLE;
      if (b & SDL_BUTTON_MASK(SDL_BUTTON_RIGHT)) return SDL_BUTTON_RIGHT;
      return 0;
    }
  };

  lua.new_enum(
    "MouseButton",
    "none",   0,
    "left",   SDL_BUTTON_LEFT,
    "middle", SDL_BUTTON_MIDDLE,
    "right",  SDL_BUTTON_RIGHT
  );

  lua.new_usertype<mouse>(
    "Mouse",
    "x", sol::property(&mouse::x),
    "y", sol::property(&mouse::y),
    "xy", &mouse::xy,
    "button", sol::readonly_property(&mouse::button)
  );

  lua["mouse"] = mouse{};

  struct keyboard final {
    static auto index(const keyboard&, sol::stack_object key, sol::this_state state) {
      static const boost::unordered_flat_map<std::string, SDL_Scancode, transparent_string_hash, std::equal_to<>> map{
        {"a", SDL_SCANCODE_A}, {"b", SDL_SCANCODE_B}, {"c", SDL_SCANCODE_C}, {"d", SDL_SCANCODE_D},
        {"e", SDL_SCANCODE_E}, {"f", SDL_SCANCODE_F}, {"g", SDL_SCANCODE_G}, {"h", SDL_SCANCODE_H},
        {"i", SDL_SCANCODE_I}, {"j", SDL_SCANCODE_J}, {"k", SDL_SCANCODE_K}, {"l", SDL_SCANCODE_L},
        {"m", SDL_SCANCODE_M}, {"n", SDL_SCANCODE_N}, {"o", SDL_SCANCODE_O}, {"p", SDL_SCANCODE_P},
        {"q", SDL_SCANCODE_Q}, {"r", SDL_SCANCODE_R}, {"s", SDL_SCANCODE_S}, {"t", SDL_SCANCODE_T},
        {"u", SDL_SCANCODE_U}, {"v", SDL_SCANCODE_V}, {"w", SDL_SCANCODE_W}, {"x", SDL_SCANCODE_X},
        {"y", SDL_SCANCODE_Y}, {"z", SDL_SCANCODE_Z},

        {"0", SDL_SCANCODE_0}, {"1", SDL_SCANCODE_1}, {"2", SDL_SCANCODE_2}, {"3", SDL_SCANCODE_3},
        {"4", SDL_SCANCODE_4}, {"5", SDL_SCANCODE_5}, {"6", SDL_SCANCODE_6}, {"7", SDL_SCANCODE_7},
        {"8", SDL_SCANCODE_8}, {"9", SDL_SCANCODE_9},

        {"f1", SDL_SCANCODE_F1}, {"f2", SDL_SCANCODE_F2}, {"f3", SDL_SCANCODE_F3}, {"f4", SDL_SCANCODE_F4},
        {"f5", SDL_SCANCODE_F5}, {"f6", SDL_SCANCODE_F6}, {"f7", SDL_SCANCODE_F7}, {"f8", SDL_SCANCODE_F8},
        {"f9", SDL_SCANCODE_F9}, {"f10", SDL_SCANCODE_F10}, {"f11", SDL_SCANCODE_F11}, {"f12", SDL_SCANCODE_F12},

        {"up", SDL_SCANCODE_UP}, {"down", SDL_SCANCODE_DOWN}, {"left", SDL_SCANCODE_LEFT}, {"right", SDL_SCANCODE_RIGHT},

        {"lshift", SDL_SCANCODE_LSHIFT}, {"rshift", SDL_SCANCODE_RSHIFT},
        {"lctrl", SDL_SCANCODE_LCTRL}, {"rctrl", SDL_SCANCODE_RCTRL},
        {"lalt", SDL_SCANCODE_LALT}, {"ralt", SDL_SCANCODE_RALT},
        {"lgui", SDL_SCANCODE_LGUI}, {"rgui", SDL_SCANCODE_RGUI},

        {"escape", SDL_SCANCODE_ESCAPE}, {"space", SDL_SCANCODE_SPACE}, {"return", SDL_SCANCODE_RETURN},
        {"enter", SDL_SCANCODE_RETURN}, {"tab", SDL_SCANCODE_TAB}, {"backspace", SDL_SCANCODE_BACKSPACE},
        {"capslock", SDL_SCANCODE_CAPSLOCK}, {"delete", SDL_SCANCODE_DELETE}, {"insert", SDL_SCANCODE_INSERT},
        {"home", SDL_SCANCODE_HOME}, {"end", SDL_SCANCODE_END}, {"pageup", SDL_SCANCODE_PAGEUP},
        {"pagedown", SDL_SCANCODE_PAGEDOWN},

        {"minus", SDL_SCANCODE_MINUS}, {"equals", SDL_SCANCODE_EQUALS}, {"leftbracket", SDL_SCANCODE_LEFTBRACKET},
        {"rightbracket", SDL_SCANCODE_RIGHTBRACKET}, {"backslash", SDL_SCANCODE_BACKSLASH},
        {"semicolon", SDL_SCANCODE_SEMICOLON}, {"apostrophe", SDL_SCANCODE_APOSTROPHE},
        {"grave", SDL_SCANCODE_GRAVE}, {"comma", SDL_SCANCODE_COMMA}, {"period", SDL_SCANCODE_PERIOD},
        {"slash", SDL_SCANCODE_SLASH},

        {"kp_0", SDL_SCANCODE_KP_0}, {"kp_1", SDL_SCANCODE_KP_1}, {"kp_2", SDL_SCANCODE_KP_2},
        {"kp_3", SDL_SCANCODE_KP_3}, {"kp_4", SDL_SCANCODE_KP_4}, {"kp_5", SDL_SCANCODE_KP_5},
        {"kp_6", SDL_SCANCODE_KP_6}, {"kp_7", SDL_SCANCODE_KP_7}, {"kp_8", SDL_SCANCODE_KP_8},
        {"kp_9", SDL_SCANCODE_KP_9}, {"kp_period", SDL_SCANCODE_KP_PERIOD}, {"kp_divide", SDL_SCANCODE_KP_DIVIDE},
        {"kp_multiply", SDL_SCANCODE_KP_MULTIPLY}, {"kp_minus", SDL_SCANCODE_KP_MINUS},
        {"kp_plus", SDL_SCANCODE_KP_PLUS}, {"kp_enter", SDL_SCANCODE_KP_ENTER}, {"kp_equals", SDL_SCANCODE_KP_EQUALS},

        {"printscreen", SDL_SCANCODE_PRINTSCREEN}, {"scrolllock", SDL_SCANCODE_SCROLLLOCK},
        {"pause", SDL_SCANCODE_PAUSE}, {"numlock", SDL_SCANCODE_NUMLOCKCLEAR},
        {"menu", SDL_SCANCODE_MENU}, {"application", SDL_SCANCODE_APPLICATION}
      };

      sol::state_view lua{state};
      const auto name = key.as<std::string_view>();
      const auto it = map.find(name);
      if (it == map.end()) [[unlikely]] {
        return sol::make_object(lua, sol::lua_nil);
      }

      const auto scancode = it->second;
      const auto pressed = SDL_GetKeyboardState(nullptr)[scancode];
      return sol::make_object(lua, pressed);
    }
  };

  lua.new_usertype<keyboard>(
    "Keyboard",
    sol::no_constructor,
    sol::meta_function::index, &keyboard::index
  );

  lua["keyboard"] = keyboard{};

  lua.new_usertype<mail>(
    "Mail",
    sol::constructors<mail(
      std::shared_ptr<entityproxy>,
      std::optional<std::shared_ptr<entityproxy>>,
      std::string_view
    )>()
  );

  lua.new_usertype<postalservice>(
    "PostalService",
    sol::constructors<postalservice()>(),
    "post", &postalservice::post
  );

  lua["postalservice"] = postalservice{};

  lua.new_usertype<timermanager>(
    "TimerManager",
    sol::no_constructor,
    "cancel", &timermanager::cancel,
    "clear", &timermanager::clear,
    "set", &timermanager::set,
    "singleshot", &timermanager::singleshot
  );

  lua.new_usertype<label>(
    "Label",
    sol::no_constructor,
    sol::base_classes, sol::bases<widget>(),
    "font", sol::property(&label::set_font),
    "set", sol::overload(
      [](
        std::shared_ptr<label> self,
        float x,
        float y
      ) {
        self->set(x, y);
      },
      [](
        std::shared_ptr<label> self,
        std::string_view text,
        float x,
        float y
      ) {
        self->set(text, x, y);
      }
    ),
    "effect", sol::writeonly_property([](label& self, sol::object argument) {
      if (argument == sol::lua_nil) {
        self.clear_effects();
        return;
      }

      auto table = argument.as<sol::table>();
      boost::unordered_flat_map<size_t, std::optional<glypheffect>> updates;

      for (const auto& [key, value] : table) {
        const auto index = key.as<size_t>() - 1;

        if (value == sol::lua_nil) {
          updates[index] = std::nullopt;
        } else {
          auto props = value.as<sol::table>();

          glypheffect effect;
          effect.xoffset = props.get_or("xoffset", .0f);
          effect.yoffset = props.get_or("yoffset", .0f);
          effect.scale = props.get_or("scale", 1.f);
          effect.r = static_cast<uint8_t>(props.get_or("r", 255.0));
          effect.g = static_cast<uint8_t>(props.get_or("g", 255.0));
          effect.b = static_cast<uint8_t>(props.get_or("b", 255.0));
          effect.alpha = static_cast<uint8_t>(props.get_or("alpha", 255.0));

          updates[index] = effect;
        }
      }

      self.set_effects(updates);
    }),
    "clear", &label::clear
  );

  lua.new_usertype<widget>(
    "Widget",
    sol::no_constructor
  );

  lua.new_usertype<fontfactory>(
    "FontFactory",
    sol::no_constructor,
    "get", &fontfactory::get
  );

  lua.new_usertype<canvas>(
    "Canvas",
    sol::no_constructor,
    "clear", &canvas::clear,
    "pixels", sol::property(&canvas::set_pixels)
  );

  std::println("Powered by Carimbo: https://carimbo.site");
  std::println("Version: {}", GIT_VERSION);
  std::println("Built on: {}, {} UTC", __DATE__, __TIME__);

  const auto jit = lua["jit"];
  if (jit.valid()) {
    std::println("Runtime: {}", jit["version"].get<std::string>());
  } else {
    std::println("Runtime: {}", lua["_VERSION"].get<std::string>());
  }

  std::println("License: MIT");
  std::println("Author: Rodrigo Delduca https://rodrigodelduca.org");

  lua.script(bootstrap, "@bootstrap");
  lua.script(debugger, "@debugger");

  const auto buffer = io::read("scripts/main.lua");
  std::string_view script{reinterpret_cast<const char*>(buffer.data()), buffer.size()};
  const auto source = lua.safe_script(script, "@main.lua");
  verify(source);

  const auto engine = lua["engine"].get<std::shared_ptr<::engine>>();

  lua["scenemanager"] = engine->scenemanager();
  lua["fontfactory"] = engine->resourcemanager()->fontfactory();
  lua["overlay"] = engine->overlay();
  lua["canvas"] = engine->canvas();
  lua["statemanager"] = engine->statemanager();

  auto viewport = lua.create_table();
  viewport["width"] = engine->window()->width();
  viewport["height"] = engine->window()->height();

  lua["viewport"] = viewport;

  const auto setup = lua["setup"].get<sol::protected_function>();
  const auto result = setup();
  verify(result);
  engine->add_loopable(std::make_shared<lua_loopable>(lua));

  const auto end = SDL_GetPerformanceCounter();
  const auto elapsed =
      (static_cast<double>(end - start) * 1000.0) / static_cast<double>(SDL_GetPerformanceFrequency());
  std::println("boot time {:.3f}ms", elapsed);

  engine->run();
}
