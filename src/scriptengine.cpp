#include "scriptengine.hpp"
#include "constant.hpp"
#include "objectproxy.hpp"
#include <sol/property.hpp>

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
  boost::static_string<48> _name;

  explicit sentinel(std::string_view name) noexcept
      : _name(name) {}

  ~sentinel() noexcept {
    std::println("[garbagecollector] collected {}", _name);
  }
};

struct metaobject {
  static sol::object index(objectproxy& self, sol::stack_object key, sol::this_state state) {
    sol::state_view lua{state};
    const auto name = key.as<std::string_view>();

    if (auto* sc = self._registry.try_get<scriptable>(self._entity); sc && sc->module.valid()) {
      const auto fname = std::format("on_{}", name);
      if (auto fn = sc->module[fname].get<sol::protected_function>(); fn.valid()) {
        return sol::make_object(lua, [fn](sol::variadic_args va) { return fn(va); });
      }
    }

    return self.kv.get(name)->value();
  }

  static void new_index(objectproxy& self, sol::stack_object key, sol::stack_object value) {
    self.kv.set(key.as<std::string_view>(), value);
  }
};

namespace {
  constexpr auto DEADZONE = 8000;

  int16_t deadzone(int16_t value) noexcept {
    if (std::abs(value) < DEADZONE) {
      return 0;
    }

    return value;
  }
}

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

  lua.new_usertype<sentinel>(
    "sentinel",
    sol::no_constructor
  );

  lua["sentinel"] = [&lua](sol::object object, sol::object name) {
    auto instance = sol::make_object<sentinel>(lua, name.as<std::string_view>());
    object.as<sol::table>().raw_set("__sentinel", instance);

    return instance;
  };

  lua["math"]["random"] = sol::overload(
    []() noexcept { return rng::global().uniform(); },
    [](lua_Integer upper) noexcept { return rng::global().range<lua_Integer>(1, upper); },
    [](lua_Integer low, lua_Integer high) noexcept { return rng::global().range<lua_Integer>(low, high); }
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

  lua["queryparam"] = [](std::string_view key, std::string_view defval) -> std::string {
#ifdef EMSCRIPTEN
    const auto script = std::format(
        R"javascript(
          new URLSearchParams(location.search).get("{}") ?? "{}"
        )javascript",
        key,
        defval
    );

    if (const auto* result = emscripten_run_script_string(script.c_str()); result && *result) {
      return std::string{result};
    }
#else
    std::array<char, 64> uppercase_key{};
    const auto len = std::min(key.size(), uppercase_key.size() - 1);
    for (std::size_t i = 0; i < len; ++i) {
      uppercase_key[i] = static_cast<char>(std::toupper(static_cast<unsigned char>(key[i])));
    }

    if (const auto* value = std::getenv(uppercase_key.data()); value && *value) {
      return std::string{value};
    }
#endif

    return std::string{defval};
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
    "subscribe", [](observable& self, sol::protected_function fn) -> uint32_t {
      return self.subscribe(std::move(fn));
    },
    "unsubscribe", [](observable& self, uint32_t id) {
      self.unsubscribe(id);
    },
    sol::meta_function::addition, [](const observable& obs, double rhs) {
      const auto value = obs.value();
      return (value.valid() ? value.as<double>() : .0) + rhs;
    },
    sol::meta_function::subtraction, [](const observable& obs, double rhs) {
      const auto value = obs.value();
      return (value.valid() ? value.as<double>() : .0) - rhs;
    },
    sol::meta_function::multiplication, [](const observable& obs, double rhs) {
      const auto value = obs.value();
      return (value.valid() ? value.as<double>() : .0) * rhs;
    },
    sol::meta_function::division, [](const observable& obs, double rhs) {
      const auto value = obs.value();
      return (value.valid() ? value.as<double>() : .0) / rhs;
    },
    sol::meta_function::modulus, [](const observable& obs, double rhs) {
      const auto value = obs.value();
      return std::fmod(value.valid() ? value.as<double>() : .0, rhs);
    },
    sol::meta_function::unary_minus, [](const observable& obs) {
      const auto value = obs.value();
      return -(value.valid() ? value.as<double>() : .0);
    },
    sol::meta_function::equal_to, [](const observable& obs, const sol::object& rhs) {
      const auto value = obs.value();
      if (!value.valid()) return !rhs.valid() || rhs.is<sol::lua_nil_t>();
      if (!rhs.valid() || rhs.is<sol::lua_nil_t>()) return false;
      if (value.get_type() != rhs.get_type()) return false;
      switch (value.get_type()) {
        case sol::type::number: return value.as<double>() == rhs.as<double>();
        case sol::type::string: return value.as<std::string_view>() == rhs.as<std::string_view>();
        case sol::type::boolean: return value.as<bool>() == rhs.as<bool>();
        default: return value == rhs;
      }
    },
    sol::meta_function::less_than, [](const observable& obs, double rhs) {
      const auto value = obs.value();
      return (value.valid() ? value.as<double>() : .0) < rhs;
    }
  );

  lua.new_enum(
    "Player",
    "one", 0,
    "two", 1,
    "three", 2,
    "four", 3
  );

  lua.new_usertype<objectproxy>(
    "Entity",
    sol::no_constructor,
    "id", sol::property(&objectproxy::id),
    "x", sol::property(&objectproxy::x, &objectproxy::set_x),
    "y", sol::property(&objectproxy::y, &objectproxy::set_y),
    "z", sol::property(&objectproxy::z, &objectproxy::set_z),
    "alpha", sol::property(&objectproxy::alpha, &objectproxy::set_alpha),
    "angle", sol::property(&objectproxy::angle, &objectproxy::set_angle),
    "scale", sol::property(&objectproxy::scale, &objectproxy::set_scale),
    "flip", sol::property(&objectproxy::flip, &objectproxy::set_flip),
    "visible", sol::property(&objectproxy::visible, &objectproxy::set_visible),
    "action", sol::property(&objectproxy::action, &objectproxy::set_action),
    "kind", sol::property(&objectproxy::kind, &objectproxy::set_kind),
    "position", sol::property(
      &objectproxy::position,
      [](objectproxy& self, sol::table table) {
        const auto x = table.get_or("x", table.get_or(1, .0f));
        const auto y = table.get_or("y", table.get_or(2, .0f));
        self.set_position({x, y});
      }
    ),
    "velocity", sol::property(
      &objectproxy::velocity,
      [](objectproxy& self, sol::table table) {
        const auto x = table.get_or("x", table.get_or(1, .0f));
        const auto y = table.get_or("y", table.get_or(2, .0f));
        self.set_velocity({x, y});
      }
    ),
    "on_hover", &objectproxy::set_onhover,
    "on_unhover", &objectproxy::set_onunhover,
    "on_touch", &objectproxy::set_ontouch,
    "on_begin", &objectproxy::set_onbegin,
    "on_end", &objectproxy::set_onend,
    "on_collision", &objectproxy::set_oncollision,
    "on_collision_end", &objectproxy::set_oncollisionend,
    "on_tick", &objectproxy::set_ontick,
    "clone", &objectproxy::clone,
    "alive", sol::property(&objectproxy::alive),
    "die", &objectproxy::die,
    "observable", [](objectproxy& self, std::string_view name) {
      return self.kv.get(name);
    },
    "subscribe", [](objectproxy& self, std::string_view name, sol::protected_function fn) -> uint32_t {
      return self.kv.get(name)->subscribe(std::move(fn));
    },
    "unsubscribe", [](objectproxy& self, std::string_view name, uint32_t id) {
      self.kv.get(name)->unsubscribe(id);
    },
    sol::meta_function::index, metaobject::index,
    sol::meta_function::new_index, metaobject::new_index
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

      self.destroy(name);
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

        if (auto fn = module["on_enter"].get<sol::protected_function>(); fn.valid()) {
          auto* const ptr = scene.get();
          const auto wrapper = [fn, ptr, &lua, module]() {
            auto pool = lua.create_table();

            ptr->populate(pool);

            lua["pool"] = pool;

            const auto result = fn();
            if (!result.valid()) {
              sol::error err = result;
              throw std::runtime_error(err.what());
            }

            if (auto onloop = module["on_loop"].get<sol::protected_function>(); onloop.valid()) {
              ptr->set_onloop(std::move(onloop));
            }

            if (auto onmotion = module["on_motion"].get<sol::protected_function>(); onmotion.valid()) {
              ptr->set_onmotion(std::move(onmotion));
            }

            if (auto oncamera = module["on_camera"].get<sol::protected_function>(); oncamera.valid()) {
              ptr->set_oncamera(std::move(oncamera));
            }

            if (auto ontext = module["on_text"].get<sol::protected_function>(); ontext.valid()) {
              ptr->set_ontext(std::move(ontext));
            }

            if (auto ontouch = module["on_touch"].get<sol::protected_function>(); ontouch.valid()) {
              ptr->set_ontouch(std::move(ontouch));
            }

            if (auto onkeypress = module["on_keypress"].get<sol::protected_function>(); onkeypress.valid()) {
              ptr->set_onkeypress(std::move(onkeypress));
            }

            if (auto onkeyrelease = module["on_keyrelease"].get<sol::protected_function>(); onkeyrelease.valid()) {
              ptr->set_onkeyrelease(std::move(onkeyrelease));
            }

            if (auto ontick = module["on_tick"].get<sol::protected_function>(); ontick.valid()) {
              ptr->set_ontick(std::move(ontick));
            }

            if (auto onleave = module["on_leave"].get<sol::protected_function>(); onleave.valid()) {
              const auto wrapper = [onleave, &lua]() {
                const auto result = onleave();
                if (!result.valid()) {
                  sol::error err = result;
                  throw std::runtime_error(err.what());
                }

                lua["pool"] = sol::lua_nil;
              };

              ptr->set_onleave(std::move(wrapper));
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

  lua.new_usertype<overlay>(
    "Overlay",
    sol::no_constructor,
    "label", sol::overload(
      sol::resolve<std::shared_ptr<::label>(std::string_view)>(&overlay::label),
      sol::resolve<void(std::shared_ptr<::label>)>(&overlay::label)
    ),
    "cursor", sol::overload(
      sol::resolve<void(std::string_view)>(&overlay::cursor),
      sol::resolve<void(std::nullptr_t)>(&overlay::cursor)
    ),
    "dispatch", &overlay::dispatch
  );

  lua.new_usertype<particleprops>(
    "ParticleProps",
    sol::no_constructor,
    "spawning", &particleprops::spawning,
    "position", sol::writeonly_property(
      [](particleprops& self, sol::table table) {
        const auto x = table.get_or("x", table.get_or(1, .0f));
        const auto y = table.get_or("y", table.get_or(2, .0f));

        self.set_position(x, y);
      }
    )
  );

  lua.new_usertype<particlepool>(
    "ParticlePool",
    sol::no_constructor,
    "clear", &particlepool::clear
  );

  lua["cassette"] = cassette();

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
    "with_ticks", &enginefactory::with_ticks,
    "width", &enginefactory::width,
    "height", &enginefactory::height,
    "create", [](enginefactory& self, sol::this_state state) {
      sol::state_view lua{state};
      auto ptr = self.create();

      lua["overlay"] = ptr->overlay();
      lua["scenemanager"] = ptr->scenemanager();
      lua["canvas"] = ptr->canvas();

      auto viewport = lua.create_table();
      viewport["width"] = self.width();
      viewport["height"] = self.height();
      lua["viewport"] = viewport;

      auto mouse = lua.create_table();

      mouse["x"] = []() noexcept -> float {
        float x, y;
        SDL_GetMouseState(&x, &y);
        SDL_RenderCoordinatesFromWindow(renderer, x, y, &x, &y);
        return x;
      };

      mouse["y"] = []() noexcept -> float {
        float x, y;
        SDL_GetMouseState(&x, &y);
        SDL_RenderCoordinatesFromWindow(renderer, x, y, &x, &y);
        return y;
      };

      mouse["xy"] = []() noexcept -> std::tuple<float, float> {
        float x, y;
        SDL_GetMouseState(&x, &y);
        SDL_RenderCoordinatesFromWindow(renderer, x, y, &x, &y);
        return {x, y};
      };

      mouse["button"] = []() noexcept -> int {
        float x, y;
        const auto button = SDL_GetMouseState(&x, &y);
        if (button & SDL_BUTTON_MASK(SDL_BUTTON_LEFT)) return SDL_BUTTON_LEFT;
        if (button & SDL_BUTTON_MASK(SDL_BUTTON_MIDDLE)) return SDL_BUTTON_MIDDLE;
        if (button & SDL_BUTTON_MASK(SDL_BUTTON_RIGHT)) return SDL_BUTTON_RIGHT;
        return 0;
      };

      lua["mouse"] = mouse;

      ptr->scenemanager()->set_runtime(lua);

      return ptr;
    }
  );

  lua.new_usertype<vec2>(
    "Vec2",
    sol::constructors<vec2(), vec2(float, float)>(),
    "x", &vec2::x,
    "y", &vec2::y,
    "zero", &vec2::zero,
    "one", &vec2::one,
    "up", &vec2::up,
    "down", &vec2::down,
    "left", &vec2::left,
    "right", &vec2::right,
    "lerp", sol::resolve<vec2(vec2 const&, vec2 const&, float)>(&lerp),
    "dot", sol::resolve<float(vec2 const&, vec2 const&)>(&dot),
    "cross", sol::resolve<float(vec2 const&, vec2 const&)>(&cross),
    "length", sol::resolve<float(vec2 const&)>(&length),
    "length_squared", sol::resolve<float(vec2 const&)>(&length_squared),
    "distance", sol::resolve<float(vec2 const&, vec2 const&)>(&distance),
    "distance_squared", sol::resolve<float(vec2 const&, vec2 const&)>(&distance_squared),
    "normalize", sol::resolve<vec2(vec2 const&)>(&normalize),
    "angle", sol::resolve<float(vec2 const&)>(&angle),
    "angle_between", sol::resolve<float(vec2 const&, vec2 const&)>(&angle_between),
    "rotate", sol::resolve<vec2(vec2 const&, float)>(&rotate),
    "perpendicular", sol::resolve<vec2(vec2 const&)>(&perpendicular),
    "reflect", sol::resolve<vec2(vec2 const&, vec2 const&)>(&reflect),
    "project", sol::resolve<vec2(vec2 const&, vec2 const&)>(&project),
    "clamp", sol::resolve<vec2(vec2 const&, vec2 const&, vec2 const&)>(&clamp),
    sol::meta_function::addition, sol::resolve<vec2(vec2 const&, vec2 const&)>(&operator+),
    sol::meta_function::subtraction, sol::resolve<vec2(vec2 const&, vec2 const&)>(&operator-),
    sol::meta_function::multiplication, sol::overload(
      sol::resolve<vec2(vec2 const&, float)>(&operator*),
      sol::resolve<vec2(vec2 const&, vec2 const&)>(&operator*)
    ),
    sol::meta_function::division, sol::resolve<vec2(vec2 const&, float)>(&operator/),
    sol::meta_function::equal_to, sol::resolve<bool(vec2 const&, vec2 const&)>(&operator==),
    sol::meta_function::unary_minus, sol::resolve<vec2(vec2 const&)>(&operator-)
  );

  lua.new_usertype<vec3>(
    "Vec3",
    sol::constructors<vec3(), vec3(float, float, float)>(),
    "x", &vec3::x,
    "y", &vec3::y,
    "z", &vec3::z
  );

  lua.new_usertype<quad>(
    "Quad",
    sol::constructors<quad(), quad(float, float, float, float)>(),
    "x", &quad::x,
    "y", &quad::y,
    "width", &quad::w,
    "height", &quad::h
  );

  lua.new_usertype<cassette>(
    "Cassette",
    sol::no_constructor,
    "clear", sol::overload(
      sol::resolve<void(std::string_view)>(&cassette::clear),
      sol::resolve<void()>(&cassette::clear)
    ),
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
          if (const auto found = self.find(key)) {
            if (const auto* v = std::get_if<std::string>(&*found)) {
              return sol::make_object(lua, *v);
            }
          }

          return sol::make_object(lua, fallback_value.as<std::string_view>());
        }

        default: {
          const auto value = self.find(key);
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

  lua.new_enum(
    "MouseButton",
    "none",   0,
    "left",   SDL_BUTTON_LEFT,
    "middle", SDL_BUTTON_MIDDLE,
    "right",  SDL_BUTTON_RIGHT
  );

  struct keyboard final {
    static auto index(const keyboard&, sol::stack_object key, sol::this_state state) {
      static const boost::unordered_flat_map<std::string_view, SDL_Scancode> mapping{
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

        {"up", SDL_SCANCODE_UP}, {"down", SDL_SCANCODE_DOWN},
        {"left", SDL_SCANCODE_LEFT}, {"right", SDL_SCANCODE_RIGHT},

        {"shift", SDL_SCANCODE_LSHIFT}, {"ctrl", SDL_SCANCODE_LCTRL},

        {"escape", SDL_SCANCODE_ESCAPE}, {"space", SDL_SCANCODE_SPACE},
        {"enter", SDL_SCANCODE_RETURN}, {"backspace", SDL_SCANCODE_BACKSPACE},
        {"tab", SDL_SCANCODE_TAB}
      };

      sol::state_view lua{state};
      const auto it = mapping.find(key.as<std::string_view>());
      if (it == mapping.end()) [[unlikely]] {
        return sol::make_object(lua, sol::lua_nil);
      }

      return sol::make_object(lua, SDL_GetKeyboardState(nullptr)[it->second]);
    }
  };

  lua.new_usertype<keyboard>(
    "Keyboard",
    sol::no_constructor,
    sol::meta_function::index, &keyboard::index
  );

  lua["keyboard"] = keyboard{};

  struct gamepadslot final {
    int slot;
    std::unique_ptr<SDL_Gamepad, SDL_Deleter> gamepad{nullptr};

    static auto index(gamepadslot& self, sol::stack_object key, sol::this_state state) {
      sol::state_view lua{state};
      const auto name = key.as<std::string_view>();

      if (name == "connected") {
        return sol::make_object(lua, self.valid());
      }

      if (name == "name") {
        if (self.valid()) [[likely]] {
          return sol::make_object(lua, SDL_GetGamepadName(self.gamepad.get()));
        }

        return sol::make_object(lua, sol::lua_nil);
      }

      if (name == "leftstick") {
        const auto x = SDL_GetGamepadAxis(self.gamepad.get(), SDL_GAMEPAD_AXIS_LEFTX);
        const auto y = SDL_GetGamepadAxis(self.gamepad.get(), SDL_GAMEPAD_AXIS_LEFTY);

        return sol::make_object(lua, std::make_pair(deadzone(x), deadzone(y)));
      }

      if (name == "rightstick") {
        const auto x = SDL_GetGamepadAxis(self.gamepad.get(), SDL_GAMEPAD_AXIS_RIGHTX);
        const auto y = SDL_GetGamepadAxis(self.gamepad.get(), SDL_GAMEPAD_AXIS_RIGHTY);

        return sol::make_object(lua, std::make_pair(deadzone(x), deadzone(y)));
      }

      if (name == "triggers") {
        const auto left = SDL_GetGamepadAxis(self.gamepad.get(), SDL_GAMEPAD_AXIS_LEFT_TRIGGER);
        const auto right = SDL_GetGamepadAxis(self.gamepad.get(), SDL_GAMEPAD_AXIS_RIGHT_TRIGGER);

        return sol::make_object(lua, std::make_pair(deadzone(left), deadzone(right)));
      }

      {
        static const boost::unordered_flat_map<std::string_view, SDL_GamepadButton> mapping{
          {"south", SDL_GAMEPAD_BUTTON_SOUTH}, {"east", SDL_GAMEPAD_BUTTON_EAST},
          {"west", SDL_GAMEPAD_BUTTON_WEST}, {"north", SDL_GAMEPAD_BUTTON_NORTH},
          {"back", SDL_GAMEPAD_BUTTON_BACK}, {"guide", SDL_GAMEPAD_BUTTON_GUIDE},
          {"start", SDL_GAMEPAD_BUTTON_START}, {"leftstick", SDL_GAMEPAD_BUTTON_LEFT_STICK},
          {"rightstick", SDL_GAMEPAD_BUTTON_RIGHT_STICK}, {"leftshoulder", SDL_GAMEPAD_BUTTON_LEFT_SHOULDER},
          {"rightshoulder", SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER}, {"up", SDL_GAMEPAD_BUTTON_DPAD_UP},
          {"down", SDL_GAMEPAD_BUTTON_DPAD_DOWN}, {"left", SDL_GAMEPAD_BUTTON_DPAD_LEFT},
          {"right", SDL_GAMEPAD_BUTTON_DPAD_RIGHT}
        };

        const auto it = mapping.find(name);
        if (it != mapping.end()) [[likely]] {
          if (self.valid()) [[likely]] {
            return sol::make_object(lua, SDL_GetGamepadButton(self.gamepad.get(), it->second));
          }
          return sol::make_object(lua, false);
        }
      }

      {
        static const boost::unordered_flat_map<std::string_view, SDL_GamepadAxis> mapping{
          {"leftx", SDL_GAMEPAD_AXIS_LEFTX}, {"lefty", SDL_GAMEPAD_AXIS_LEFTY},
          {"rightx", SDL_GAMEPAD_AXIS_RIGHTX}, {"righty", SDL_GAMEPAD_AXIS_RIGHTY},
          {"triggerleft", SDL_GAMEPAD_AXIS_LEFT_TRIGGER}, {"triggerright", SDL_GAMEPAD_AXIS_RIGHT_TRIGGER}
        };

        const auto it = mapping.find(name);
        if (it != mapping.end()) {
          if (self.valid()) [[likely]] {
            const auto value = SDL_GetGamepadAxis(self.gamepad.get(), it->second);
            return sol::make_object(lua, deadzone(value));
          }

          return sol::make_object(lua, 0);
        }
      }

      return sol::make_object(lua, sol::lua_nil);
    }

    [[nodiscard]] bool valid() noexcept {
      if (gamepad) [[likely]] {
        if (SDL_GamepadConnected(gamepad.get())) [[likely]] {
          return true;
        }

        gamepad.reset();
      }

      if (!SDL_HasGamepad()) {
        return false;
      }

      auto count = 0;
      const auto gamepads = std::unique_ptr<SDL_JoystickID[], SDL_Deleter>(SDL_GetGamepads(&count));
      if (gamepads && slot < count) [[likely]] {
        gamepad.reset(SDL_OpenGamepad(gamepads[static_cast<size_t>(slot)]));
      }

      return gamepad != nullptr;
    }
  };

  std::array<gamepadslot, 4> gamepadslots{gamepadslot{0}, gamepadslot{1}, gamepadslot{2}, gamepadslot{3}};

  struct gamepads final {
    [[nodiscard]] static int count() noexcept {
      auto total = 0;
      SDL_GetGamepads(&total);
      return std::min(total, 4);
    }
  };

  lua.new_usertype<gamepadslot>(
    "GamepadSlot",
    sol::no_constructor,
    sol::meta_function::index, &gamepadslot::index
  );

  lua.new_usertype<gamepads>(
    "Gamepads",
    sol::no_constructor,
    "count", sol::property(&gamepads::count),
    sol::meta_function::index, [&gamepadslots](gamepads&, int slot) noexcept -> gamepadslot& {
      return gamepadslots[static_cast<size_t>(std::clamp(slot, 0, 3))];
    }
  );

  lua["gamepads"] = gamepads{};

  lua.new_usertype<label>(
    "Label",
    sol::no_constructor,
    sol::base_classes, sol::bases<widget>(),
    "clear", &label::clear,
    "effect", sol::writeonly_property([](label& self, sol::object argument) {
      if (argument == sol::lua_nil) {
        self.clear_effects();
        return;
      }

      const auto table = argument.as<sol::table>();
      boost::unordered_flat_map<size_t, std::optional<glypheffect>> updates;
      updates.reserve(table.size());

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
    "glyphs", sol::property(&label::glyphs),
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
    )
  );

  lua.new_usertype<widget>(
    "Widget",
    sol::no_constructor
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

  const auto setup = lua["setup"].get<sol::protected_function>();
  const auto result = setup();
  verify(result);
  engine->add_loopable(std::make_shared<lua_loopable>(lua));

  const auto end = SDL_GetPerformanceCounter();
  const auto elapsed =
      (static_cast<double>(end - start) * 1000.0) / static_cast<double>(SDL_GetPerformanceFrequency());
  std::println("boot time {:.3f}ms", elapsed);

  engine->run();

  lua.collect_garbage();
  lua.collect_garbage();
}
