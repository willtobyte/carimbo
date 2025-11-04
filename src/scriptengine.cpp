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

static sol::object searcher(sol::this_state state, const std::string& module) {
  sol::state_view lua{state};

  const auto filename = std::format("scripts/{}.lua", module);
  const auto buffer = storage::io::read(filename);
  std::string_view script{reinterpret_cast<const char*>(buffer.data()), buffer.size()};

  const auto loader = lua.load(script, std::format("@{}", filename));
  if (!loader.valid()) [[unlikely]] {
    sol::error err = loader;
    throw std::runtime_error(err.what());
  }

  return sol::make_object(lua, loader.get<sol::protected_function>());
}

class lua_loopable final : public framework::loopable {
public:
  explicit lua_loopable(const sol::state_view lua) : _L(lua) {}

  void loop(float delta) override {
    _frames++;
    const auto now = SDL_GetTicks();
    _elapsed += now - _start;
    _start = now;

    const auto memory = lua_gc(_L, LUA_GCCOUNT, 0);

    if (_elapsed >= 1000) [[unlikely]] {
      std::println("{:.1f} {}KB", (1000.0 * static_cast<double>(_frames)) / static_cast<double>(_elapsed), memory);

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
  uint64_t _start{SDL_GetTicks()};
};

static sol::object _to_lua(const nlohmann::json& value, sol::state_view lua) {
  using nlohmann::json;

  switch (value.type()) {
    case json::value_t::object: {
      auto table = lua.create_table(0, static_cast<int>(value.size()));
      for (const auto& [k, v] : value.items()) {
        table[k] = _to_lua(v, lua);
      }

      return sol::make_object(lua, table);
    }

    case json::value_t::array: {
      auto table = lua.create_table(static_cast<int>(value.size()), 0);
      for (auto i = 0uz, n = value.size(); i < n; ++i) {
        table[i + 1] = _to_lua(value[i], lua);
      }

      return sol::make_object(lua, table);
    }

    case json::value_t::string:
      return sol::make_object(lua, value.get<std::string>());

    case json::value_t::boolean:
      return sol::make_object(lua, value.get<bool>());

    case json::value_t::number_integer:
      return sol::make_object(lua, value.get<int64_t>());

    case json::value_t::number_unsigned:
      return sol::make_object(lua, value.get<uint64_t>());

    case json::value_t::number_float:
      return sol::make_object(lua, value.get<double>());

    default:
      return sol::make_object(lua, sol::lua_nil);
  }
}

static nlohmann::json _to_json(const sol::object& value) {
  using nlohmann::json;

  switch (value.get_type()) {
    case sol::type::string:
      return json(value.as<std::string>());

    case sol::type::boolean:
      return json(value.as<bool>());

    case sol::type::number: {
      const double x = value.as<double>();
      double i{};
      const double frac = std::modf(x, &i);

      if (std::fabs(frac) < std::numeric_limits<double>::epsilon()) {
        return json(static_cast<int64_t>(i));
      }

      return json(x);
    }

    case sol::type::table: {
      const auto table = value.as<sol::table>();
      const auto n = table.size();

      auto is_array = true;
      for (auto i = 1uz; i <= n; ++i) {
        if (!table[i].valid()) {
          is_array = false;
          break;
        }
      }

      if (is_array) {
        auto j = json::array();
        j.get_ref<json::array_t&>().reserve(n);
        for (auto i = 1uz; i <= n; ++i) {
          j.emplace_back(_to_json(table.get<sol::object>(i)));
        }

        return j;
      }

      auto j = json::object();
      for (const auto& kv : table) {
        j.emplace(kv.first.as<std::string>(), _to_json(kv.second));
      }

      return j;
    }

    default:
      return json();
  }
}

struct sentinel final {
  std::string _name;

  explicit sentinel(std::string name)
      : _name(std::move(name)) {}

  ~sentinel() noexcept {
    std::println("[garbagecollector] collected {}", _name);
  }
};

void framework::scriptengine::run() {
  const auto start = SDL_GetPerformanceCounter();

  sol::state lua;

  lua.open_libraries();

  lua["searcher"] = &searcher;

  const auto inject = std::format(R"lua(
    local list = package.searchers or package.loaders
    table.insert(list, searcher)
  )lua");

  lua.script(inject);

  lua["sentinel"] = [&lua](sol::object object, sol::object name) {
    auto u = sol::make_object<sentinel>(lua, name.as<std::string>());
    object.as<sol::table>().raw_set("__sentinel", u);

    return u;
  };

  lua["_"] = &localization::text;

  lua["moment"] = &moment;

  lua["openurl"] = [](const std::string& url) {
#ifdef EMSCRIPTEN
    const auto script = std::format(R"javascript(window.open('{}', '_blank', 'noopener,noreferrer');)javascript", url);
    emscripten_run_script(script.c_str());
#else
    SDL_OpenURL(url.c_str());
#endif
  };

  lua["queryparam"] = [](const std::string& key, const std::string& defval) {
    auto out = defval;

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
    auto uppercase_key = key;
    std::ranges::transform(
      uppercase_key,
      uppercase_key.begin(),
      [](unsigned char c) {
        return static_cast<char>(std::toupper(c));
      });

    if (const auto* value = std::getenv(uppercase_key.c_str()); value && *value) {
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

  platform::desktop desktop;

  lua.new_usertype<platform::desktop>(
    "Desktop",
    "folder", &platform::desktop::folder
  );

  lua["desktop"] = &desktop;

  platform::operatingsystem operatingsystem;

  lua.new_usertype<platform::operatingsystem>(
    "OperatingSystem",
    "compute", &platform::operatingsystem::compute,
    "memory", &platform::operatingsystem::memory,
    "name", &platform::operatingsystem::name
  );

  lua["operatingsystem"] = &operatingsystem;

  lua["JSON"] = lua.create_table_with(
    "parse", [](const std::string& json, sol::this_state state) {
      const auto& j = nlohmann::json::parse(json);

      sol::state_view lua(state);

      return _to_lua(j, lua);
    },
    "stringify", [](const sol::table& table) {
      nlohmann::json result;
      for (const auto& pair : table) {
        result[pair.first.as<std::string>()] = _to_json(pair.second);
      }

      return result.dump();
    }
  );

  lua.new_usertype<audio::soundfx>(
    "SoundFX",
    sol::no_constructor,
     "play", [](audio::soundfx& self, std::optional<bool> loop_opt) {
       auto loop = loop_opt.value_or(false);
       self.play(loop);
     },
     "stop", &audio::soundfx::stop,
     "volume", sol::property(&audio::soundfx::volume, &audio::soundfx::set_volume),
     "on_begin", &audio::soundfx::set_onbegin,
     "on_end", &audio::soundfx::set_onend
  );

  lua.new_usertype<audio::soundmanager>(
    "SoundManager",
    sol::no_constructor,
    "play", [](
      audio::soundmanager& self,
      const std::string& name,
      std::optional<bool> loop_opt
    ) {
      auto loop = loop_opt.value_or(false);
      self.play(name, loop);
    },
    "stop", &audio::soundmanager::stop
  );

  lua.new_enum(
    "SceneType",
    "object", framework::scenetype::object,
    "effect", framework::scenetype::effect,
    "particle", framework::scenetype::particle
  );

  lua.new_enum(
    "Controller",
    "up", input::event::gamepad::button::up,
    "down", input::event::gamepad::button::down,
    "left", input::event::gamepad::button::left,
    "right", input::event::gamepad::button::right,
    "north", input::event::gamepad::button::north,
    "east", input::event::gamepad::button::east,
    "south", input::event::gamepad::button::south,
    "west", input::event::gamepad::button::west
  );

  lua.new_enum(
    "Reflection",
    "none", graphics::reflection::none,
    "horizontal", graphics::reflection::horizontal,
    "vertical", graphics::reflection::vertical,
    "both", graphics::reflection::both
  );

  struct metaobject {
    static sol::object index(framework::object& self, sol::stack_object key, sol::this_state state) {
      auto& store = self.kv();
      const auto& ptr = store.get(key.as<std::string>());
      return sol::make_object(state, std::ref(*ptr));
    }

    static void new_index(framework::object& self, sol::stack_object key, sol::stack_object value) {
      auto& store = self.kv();
      store.set(key.as<std::string>(), value);
    }
  };

  lua.new_usertype<memory::observable>(
    "Observable",
    sol::no_constructor,
    "value", sol::property(&memory::observable::value),
    "set", &memory::observable::set,
    "subscribe", &memory::observable::subscribe,
    "unsubscribe", &memory::observable::unsubscribe
  );

  lua.new_usertype<framework::object>(
    "Object",
    sol::no_constructor,
    "id", sol::property(&framework::object::id),
    "kind", sol::property(&framework::object::kind),
    "x", sol::property(&framework::object::x, &framework::object::set_x),
    "y", sol::property(&framework::object::y, &framework::object::set_y),
    "alpha", sol::property(&framework::object::alpha, &framework::object::set_alpha),
    "scale", sol::property(&framework::object::scale, &framework::object::set_scale),
    "angle", sol::property(&framework::object::angle, &framework::object::set_angle),
    "reflection", sol::property(&framework::object::reflection, &framework::object::set_reflection),
    "visible", sol::property(&framework::object::visible, &framework::object::set_visible),
    "on_begin", &framework::object::set_onbegin,
    "on_end", &framework::object::set_onend,
    "on_mail", &framework::object::set_onmail,
    "on_touch", &framework::object::set_ontouch,
    "on_hover", &framework::object::set_onhover,
    "on_unhover", &framework::object::set_onunhover,
    "on_collision", &framework::object::set_oncollision,
    "action", sol::property(&framework::object::action, &framework::object::set_action),
    "placement", sol::property(
      [](framework::object& o) {
        return o.placement();
      },
      [](framework::object& self, sol::table table) {
        const auto x = table.get_or("x", table.get_or(1, .0f));
        const auto y = table.get_or("y", table.get_or(2, .0f));

        self.set_placement(x, y);
      }
    ),
    sol::meta_function::index, metaobject::index,
    sol::meta_function::new_index, metaobject::new_index
  );

  lua.new_usertype<framework::objectmanager>(
    "ObjectManager",
    sol::no_constructor,
    "create", [](
      framework::objectmanager& self,
      const std::string& kind,
      sol::optional<std::string> scope_opt,
      sol::optional<bool> manage_opt
    ) {
      const auto manage = manage_opt.value_or(true);
      if (!scope_opt) {
        return self.create(kind, std::nullopt, manage);
      }

      return self.create(kind, std::cref(*scope_opt), manage);
    },
    "clone", &framework::objectmanager::clone,
    "remove", &framework::objectmanager::remove
  );

  lua.new_usertype<framework::resourcemanager>(
    "ResourceManager",
    sol::no_constructor,
    "flush", [&lua](framework::resourcemanager& self) {
      lua.collect_garbage();
      self.flush();
    },
    "prefetch", sol::overload(
        [](framework::resourcemanager& self) {
          self.prefetch();
        },

        [](framework::resourcemanager& self, sol::table table) {
          std::vector<std::string> filenames;
          filenames.reserve(table.size());
          for (const auto& [key, value] : table) {
            filenames.emplace_back(value.as<std::string>());
          }
          self.prefetch(std::move(filenames));
        },

        [](framework::resourcemanager& self, sol::variadic_args arguments) {
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
    const framework::statemanager& e;

    bool on(input::event::gamepad::button type) {
      return e.on(index, type);
    }
  };

  lua.new_usertype<playerwrapper>(
    "PlayerWrapper",
    sol::no_constructor,
    "on", &playerwrapper::on
  );

  lua.new_enum(
    "Player",
    "one", input::event::player::one,
    "two", input::event::player::two
  );

  lua.new_usertype<framework::statemanager>(
    "StateManager",
    sol::no_constructor,
    "collides", &statemanager::collides,
    "players", sol::property(&statemanager::players),
    "player", [cache = std::make_shared<std::unordered_map<uint8_t, playerwrapper>>()](
      framework::statemanager& self,
      input::event::player player
    ) mutable {
      auto index = static_cast<uint8_t>(player);
      auto it = cache->find(index);
      if (it == cache->end()) [[unlikely]] {
        it = cache->emplace(index, playerwrapper{static_cast<uint8_t>(player), self}).first;
      }

      return it->second;
    }
  );

  lua.new_usertype<framework::scene>(
    "Scene",
    sol::no_constructor,
    "name", sol::property(&framework::scene::name)
  );

  lua.new_usertype<framework::scenemanager>(
    "SceneManager",
    sol::no_constructor,
    "current", sol::property(&framework::scenemanager::current),
    "set", &framework::scenemanager::set,
    "get", &framework::scenemanager::get,
    "destroy", [&lua](
      framework::scenemanager& self,
      const std::string& name
    ) {
      const auto scenes = self.destroy(name);

      lua.collect_garbage();
      lua.collect_garbage();

      auto loaded = lua["package"]["loaded"];
      for (const auto& scene : scenes) {
        loaded[std::format("scenes/{}", scene)] = sol::lua_nil;
      }

      lua.collect_garbage();
      lua.collect_garbage();
    },
    "register", [&lua](
      framework::scenemanager& self,
      const std::string& name
    ) {
      const auto start = SDL_GetPerformanceCounter();
      const auto scene = self.load(name);
      if (!scene) [[unlikely]] {
        return;
      }

      const auto filename = std::format("scenes/{}.lua", name);
      const auto buffer = storage::io::read(filename);
      std::string_view script{reinterpret_cast<const char*>(buffer.data()), buffer.size()};
      const auto result = lua.load(script, std::format("@{}", filename));
      const auto pf = result.get<sol::protected_function>();
      const auto exec = pf();
      if (!exec.valid()) [[unlikely]] {
        sol::error err = exec;
        throw std::runtime_error(err.what());
      }

      auto module = exec.get<sol::table>();

      auto loaded = lua["package"]["loaded"];
      loaded[std::format("scenes/{}", name)] = module;
      auto ptr = std::weak_ptr<framework::scene>(scene);

      module["get"] = [ptr, name](sol::table, const std::string& id, framework::scenetype type) {
        auto scene = ptr.lock();
        if (!scene) {
          throw std::runtime_error(std::format(
            "[scriptengine] scene {} expired while accessing object {}",
              name,
              id
          ));
        }

        return scene->get(id, type);
      };

      if (auto fn = module["on_enter"].get<sol::protected_function>(); fn.valid()) {
        auto sfn = [fn, &lua]() mutable {
          sol::protected_function_result result = fn();
          if (!result.valid()) [[unlikely]] {
            sol::error err = result;
            throw std::runtime_error(err.what());
          }

          lua.collect_garbage();
          lua.collect_garbage();
        };

        scene->set_onenter(std::move(sfn));
      }

      if (auto fn = module["on_loop"].get<sol::protected_function>(); fn.valid()) {
        auto sfn = [fn](float delta) mutable {
          sol::protected_function_result result = fn(delta);
          if (!result.valid()) [[unlikely]] {
            sol::error err = result;
            throw std::runtime_error(err.what());
          }
        };

        scene->set_onloop(std::move(sfn));
      }

      if (auto fn = module["on_text"].get<sol::protected_function>(); fn.valid()) {
        auto sfn = [fn](const std::string& text) mutable {
          sol::protected_function_result result = fn(text);
          if (!result.valid()) [[unlikely]] {
            sol::error err = result;
            throw std::runtime_error(err.what());
          }
        };

        scene->set_ontext(std::move(sfn));
      }

      if (auto fn = module["on_touch"].get<sol::protected_function>(); fn.valid()) {
        auto sfn = [fn](float x, float y) mutable {
          sol::protected_function_result result = fn(x, y);
          if (!result.valid()) [[unlikely]] {
            sol::error err = result;
            throw std::runtime_error(err.what());
          }
        };

        scene->set_ontouch(std::move(sfn));
      }

      if (auto fn = module["on_keypress"].get<sol::protected_function>(); fn.valid()) {
        auto sfn = [fn](int32_t code) mutable {
          sol::protected_function_result result = fn(code);
          if (!result.valid()) [[unlikely]] {
            sol::error err = result;
            throw std::runtime_error(err.what());
          }
        };

        scene->set_onkeypress(std::move(sfn));
      }

      if (auto fn = module["on_keyrelease"].get<sol::protected_function>(); fn.valid()) {
        auto sfn = [fn](int32_t code) mutable {
          sol::protected_function_result result = fn(code);
          if (!result.valid()) [[unlikely]] {
            sol::error err = result;
            throw std::runtime_error(err.what());
          }
        };

        scene->set_onkeyrelease(std::move(sfn));
      }

      if (auto fn = module["on_motion"].get<sol::protected_function>(); fn.valid()) {
        auto sfn = [fn](float x, float y) mutable {
          sol::protected_function_result result = fn(x, y);
          if (!result.valid()) [[unlikely]] {
            sol::error err = result;
            throw std::runtime_error(err.what());
          }
        };

        scene->set_onmotion(std::move(sfn));
      }

      if (auto fn = module["on_leave"].get<sol::protected_function>(); fn.valid()) {
        auto sfn = [fn, &lua]() mutable {
          sol::protected_function_result result = fn();
          if (!result.valid()) [[unlikely]] {
            sol::error err = result;
            throw std::runtime_error(err.what());
          }

          lua.collect_garbage();
          lua.collect_garbage();
        };

        scene->set_onleave(std::move(sfn));

        lua.collect_garbage();
        lua.collect_garbage();
      }

      const auto end = SDL_GetPerformanceCounter();
      const auto elapsed = static_cast<double>(end - start) * 1000.0 / static_cast<double>(SDL_GetPerformanceFrequency());
      std::println("[scriptengine] {} took {:.3f}ms", name, elapsed);
    }
  );

  lua.new_enum(
    "WidgetType",
    "cursor", graphics::widgettype::cursor,
    "label", graphics::widgettype::label
  );

  struct cursorproxy {
    graphics::overlay& o;

    void set(const std::string& name) { o.set_cursor(name); }

    void hide() { o.hide(); }
  };

  lua.new_usertype<cursorproxy>(
    "CursorProxy",
    sol::no_constructor,
    "set", &cursorproxy::set,
    "hide", &cursorproxy::hide
  );

  lua.new_usertype<graphics::overlay>(
    "Overlay",
    sol::no_constructor,
    "create", &graphics::overlay::create,
    "destroy", &graphics::overlay::destroy,
    "dispatch", &graphics::overlay::dispatch,
    "cursor", sol::property([](graphics::overlay& o) { return cursorproxy{o}; })
  );

  lua.new_usertype<graphics::particleprops>(
    "ParticleProps",
    sol::no_constructor,
    "active", sol::property(&graphics::particleprops::active),
    "emitting", sol::property(&graphics::particleprops::emitting),
    "placement", sol::property(
      []() {
        return nullptr;
      },
      [](graphics::particleprops& self, sol::table table) {
        const auto x = table.get_or("x", table.get_or(1, .0f));
        const auto y = table.get_or("y", table.get_or(2, .0f));

        self.set_placement(x, y);
      }
    )
  );

  lua.new_usertype<graphics::particlefactory>(
    "ParticleFactory",
    sol::no_constructor,
    "create", &graphics::particlefactory::create
  );

  lua.new_usertype<graphics::particlesystem>(
    "ParticleSystem",
    sol::no_constructor,
    "factory", sol::property(&graphics::particlesystem::factory),
    "add", &graphics::particlesystem::add,
    "clear", &graphics::particlesystem::clear,
    "set", &graphics::particlesystem::set
  );

  lua.new_usertype<framework::engine>(
    "Engine",
    sol::no_constructor,
    "canvas", &framework::engine::canvas,
    "cassette", &framework::engine::cassette,
    "objectmanager", &framework::engine::objectmanager,
    "fontfactory", &framework::engine::fontfactory,
    "overlay", &framework::engine::overlay,
    "resourcemanager", &framework::engine::resourcemanager,
    "soundmanager", &framework::engine::soundmanager,
    "statemanager", &framework::engine::statemanager,
    "scenemanager", &framework::engine::scenemanager,
    "timermanager", &framework::engine::timermanager,
    "particlesystem", &framework::engine::particlesystem,
    "run", &framework::engine::run
  );

  lua.new_enum(
    "FontEffect",
    "fadein", graphics::fonteffect::type::fadein
  );

  lua.new_usertype<graphics::font>(
    "Font",
    sol::no_constructor,
    "glyphs", sol::property(&graphics::font::glyphs)
  );

  lua.new_usertype<framework::enginefactory>(
    "EngineFactory",
    sol::constructors<framework::enginefactory()>(),
    "with_title", &framework::enginefactory::with_title,
    "with_width", &framework::enginefactory::with_width,
    "with_height", &framework::enginefactory::with_height,
    "with_scale", &framework::enginefactory::with_scale,
    "with_gravity", &framework::enginefactory::with_gravity,
    "with_fullscreen", &framework::enginefactory::with_fullscreen,
    "with_sentry", &framework::enginefactory::with_sentry,
    "create", &framework::enginefactory::create
  );

  lua.new_usertype<geometry::point>(
    "Point",
    sol::constructors<geometry::point(), geometry::point(float, float)>(),
    "set", &geometry::point::set,
    "x", sol::property(&geometry::point::x, &geometry::point::set_x),
    "y", sol::property(&geometry::point::y, &geometry::point::set_y)
  );

  lua.new_usertype<geometry::size>(
    "Size",
    sol::constructors<geometry::size(), geometry::size(float, float), geometry::size(const geometry::size&)>(),
    "width", sol::property(&geometry::size::width, &geometry::size::set_width),
    "height", sol::property(&geometry::size::height, &geometry::size::set_height)
  );

  lua.new_usertype<storage::cassette>(
    "Cassette",
    sol::no_constructor,
    "clear", &storage::cassette::clear,
    "set", [](
      storage::cassette& self,
      const std::string& key,
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
          self.set(key, value.as<std::string>());
          break;
        default:
          self.set(key, nullptr);
          break;
      }
    },
    "get", [](
      const storage::cassette& self,
      const std::string& key,
      sol::object default_value,
      sol::this_state state
    ) {
      sol::state_view lua(state);

      const nlohmann::json j = self.get<nlohmann::json>(key, _to_json(default_value));

      switch (j.type()) {
        case nlohmann::json::value_t::null:
          return sol::make_object(lua, nullptr);
        case nlohmann::json::value_t::number_integer:
          return sol::make_object(lua, j.get<int64_t>());
        case nlohmann::json::value_t::number_unsigned:
          return sol::make_object(lua, j.get<uint64_t>());
        case nlohmann::json::value_t::number_float:
          return sol::make_object(lua, j.get<double>());
        case nlohmann::json::value_t::boolean:
          return sol::make_object(lua, j.get<bool>());
        case nlohmann::json::value_t::string:
          return sol::make_object(lua, j.get<std::string>());
        default:
          return sol::make_object(lua, nullptr);
      }
    }
  );

  lua.new_usertype<graphics::color>(
    "Color",
    "color", sol::constructors<graphics::color(const std::string& )>(),

    "r", sol::property(&graphics::color::r, &graphics::color::set_r),
    "g", sol::property(&graphics::color::g, &graphics::color::set_g),
    "b", sol::property(&graphics::color::b, &graphics::color::set_b),
    "a", sol::property(&graphics::color::a, &graphics::color::set_a),

    sol::meta_function::equal_to, &graphics::color::operator==
  );

  lua.new_enum(
    "KeyEvent",
    "up", input::event::keyboard::key::up,
    "left", input::event::keyboard::key::left,
    "down", input::event::keyboard::key::down,
    "right", input::event::keyboard::key::right,
    "space", input::event::keyboard::key::space,

    "backspace", input::event::keyboard::key::backspace,
    "enter", input::event::keyboard::key::enter,
    "escape", input::event::keyboard::key::escape
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

  lua.new_enum("MouseButton",
    "none",   0,
    "left",   SDL_BUTTON_LEFT,
    "middle", SDL_BUTTON_MIDDLE,
    "right",  SDL_BUTTON_RIGHT
  );

  lua.new_usertype<mouse>("Mouse",
    "x", sol::property(&mouse::x),
    "y", sol::property(&mouse::y),
    "xy", &mouse::xy,
    "button", sol::property(&mouse::button)
  );

  lua["mouse"] = mouse{};

  struct keyboard final {
    static auto index(const keyboard&, sol::stack_object key, sol::this_state state) -> sol::object {
      static const std::unordered_map<std::string, SDL_Scancode> map{
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
      const auto name = key.as<std::string>();
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

  lua.new_usertype<framework::mail>(
    "Mail",
    sol::constructors<framework::mail(
      std::shared_ptr<framework::object>,
      std::optional<std::shared_ptr<framework::object>>,
      const std::string&
    )>()
  );

  lua.new_usertype<framework::postalservice>(
    "PostalService",
    sol::constructors<framework::postalservice()>(),
    "post", &framework::postalservice::post
  );

  lua.new_usertype<framework::timermanager>(
    "TimerManager",
    sol::no_constructor,
    "set", &framework::timermanager::set,
    "singleshot", &framework::timermanager::singleshot,
    "cancel", &framework::timermanager::cancel,
    "clear", &framework::timermanager::clear
  );

  lua.new_usertype<graphics::label>(
    "Label",
    sol::no_constructor,
    sol::base_classes, sol::bases<graphics::widget>(),
    "font", sol::property(&graphics::label::set_font),
    "set", sol::overload(
      [](
        std::shared_ptr<graphics::label> self,
        float x,
        float y
      ) {
        self->set(x, y);
      },
      [](
        std::shared_ptr<graphics::label> self,
        const std::string& text,
        float x,
        float y
      ) {
        self->set(text, x, y);
      }
    ),
    "effect", sol::property(&graphics::label::set_effect),
    "clear", &graphics::label::clear
  );

  lua.new_usertype<graphics::widget>(
    "Widget",
    sol::no_constructor
  );

  lua.new_usertype<graphics::fontfactory>(
    "FontFactory",
    sol::no_constructor,
    "get", &graphics::fontfactory::get
  );

  lua.new_usertype<graphics::canvas>(
    "Canvas",
    sol::no_constructor,
    "clear", &graphics::canvas::clear,
    "pixels", sol::property(
      [](const graphics::canvas&) {
        return nullptr;
      },
      [](graphics::canvas& self, const char* data) {
        self.set_pixels(data);
      }
    )
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

  const auto buffer = storage::io::read("scripts/main.lua");
  std::string_view script{reinterpret_cast<const char*>(buffer.data()), buffer.size()};
  const auto source = lua.safe_script(script, &sol::script_pass_on_error);
  if (!source.valid()) [[unlikely]] {
    sol::error err = source;
    throw std::runtime_error(err.what());
  }

  const auto engine = lua["engine"].get<std::shared_ptr<framework::engine>>();
  lua["canvas"] = engine->canvas();
  lua["cassette"] = engine->cassette();
  lua["fontfactory"] = engine->fontfactory();
  lua["objectmanager"] = engine->objectmanager();
  lua["overlay"] = engine->overlay();
  lua["particlesystem"] = engine->particlesystem();
  lua["resourcemanager"] = engine->resourcemanager();
  lua["postalservice"] = engine->postalservice();
  lua["scenemanager"] = engine->scenemanager();
  lua["soundmanager"] = engine->soundmanager();
  lua["statemanager"] = engine->statemanager();
  lua["timermanager"] = engine->timermanager();

  const auto setup = lua["setup"].get<sol::protected_function>();
  const auto result = setup();
  if (!result.valid()) [[unlikely]] {
    sol::error err = result;
    throw std::runtime_error(err.what());
  }

  engine->add_loopable(std::make_shared<lua_loopable>(lua));

  const auto end = SDL_GetPerformanceCounter();
  const auto elapsed =
      (static_cast<double>(end - start) * 1000.0) / static_cast<double>(SDL_GetPerformanceFrequency());
  std::println("boot time {:.3f}ms", elapsed);

  engine->run();
}
