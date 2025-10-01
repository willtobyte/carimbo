#include "scriptengine.hpp"
#include "canvas.hpp"
#include "environment.hpp"
#include "soundmanager.hpp"
#include <sol/object.hpp>
#include <sol/property.hpp>
#include <sol/types.hpp>
#include <string>

[[noreturn]] static void panic(sol::optional<std::string> maybe_message) {
  throw std::runtime_error(
    std::format("Lua panic: {}",
      maybe_message.value_or("unknown Lua error")));
}

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
  explicit lua_loopable(const sol::state_view lua, sol::function function)
      : _L(lua),
        _function(std::move(function)) {}

  void loop(float_t delta) override {
    _function(delta);

    _frames++;
    const auto now = SDL_GetTicks();
    _elapsed += now - _start;
    _start = now;

    const auto memory = lua_gc(_L, LUA_GCCOUNT, 0);

    if (_elapsed >= 1000) [[unlikely]] {
      std::println("{:.1f} {}KB", (1000.0 * static_cast<double_t>(_frames)) / static_cast<double_t>(_elapsed), memory);

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
  sol::function _function;
  uint64_t _frames{0};
  uint64_t _elapsed{0};
  uint64_t _start{SDL_GetTicks()};
};

static sol::object _to_lua(const nlohmann::json& value, sol::state_view lua) {
  using nlohmann::json;

  switch (value.type()) {
    case json::value_t::object: {
      auto t = lua.create_table(0, static_cast<int>(value.size()));
      for (const auto& [k, v] : value.items()) t[k] = _to_lua(v, lua);
      return sol::make_object(lua, t);
    }

    case json::value_t::array: {
      auto t = lua.create_table(static_cast<int>(value.size()), 0);
      for (auto i = 0uz, n = value.size(); i < n; ++i) t[i + 1] = _to_lua(value[i], lua);
      return sol::make_object(lua, t);
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
      return sol::make_object(lua, value.get<double_t>());

    default:
      return sol::make_object(lua, sol::lua_nil);
  }
}

static nlohmann::json _to_json(const sol::object& value) {
  using nlohmann::json;

  switch (value.get_type()) {
    case sol::type::table: {
      const auto table = value.as<sol::table>();

      const bool is_array =
        std::ranges::all_of(table, [](const auto& pair) {
          return pair.first.get_type() == sol::type::number
                 && pair.first.template as<size_t>() >= 1;
        });

      if (is_array) {
        auto j = json::array();
        j.get_ref<json::array_t&>().reserve(table.size());
        for (const auto& pair : table) j.emplace_back(_to_json(pair.second));
        return j;
      }

      auto j = json::object();
      for (const auto& pair : table)
        j[pair.first.as<std::string>()] = _to_json(pair.second);
      return j;
    }

    case sol::type::string:
      return json(value.as<std::string>());

    case sol::type::boolean:
      return json(value.as<bool>());

    case sol::type::number: {
      const auto number = value.as<double_t>();
      double_t intpart{};
      const auto frac = std::modf(number, &intpart);

      if (std::abs(frac) < std::numeric_limits<double_t>::epsilon())
        return json(static_cast<int64_t>(intpart));

      return json(number);
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

  sol::state lua(sol::c_call<decltype(&panic), &panic>);

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
      std::ranges::transform(uppercase_key, uppercase_key.begin(),
                            [](unsigned char c) { return static_cast<char>(std::toupper(c)); });

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
     "play", [](audio::soundfx& sfx, std::optional<bool> loop_opt) {
       auto loop = loop_opt.value_or(false);
       sfx.play(loop);
     },
     "stop", &audio::soundfx::stop,
     "on_begin", &audio::soundfx::set_onbegin,
     "on_end", &audio::soundfx::set_onend
  );

  lua.new_usertype<audio::soundmanager>(
    "SoundManager",
    sol::no_constructor,
    "play", [](
      audio::soundmanager& manager,
      const std::string& name,
      std::optional<bool> loop_opt
    ) {
      auto loop = loop_opt.value_or(false);
      manager.play(name, loop);
    },
    "stop", &audio::soundmanager::stop
  );

  lua.new_enum(
    "SceneType",
    "object", framework::scenetype::object,
    "effect", framework::scenetype::effect
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

  struct velocityproxy {
    framework::object& o;

    float_t x() const { return o.velocity().x(); }
    float_t y() const { return o.velocity().y(); }

    void set_x(float_t x) {
      auto v = o.velocity();
      v.set_x(x);
      o.set_velocity(v);
    }

    void set_y(float_t y) {
      auto v = o.velocity();
      v.set_y(y);
      o.set_velocity(v);
    }

    void set(const sol::table& table) {
      const auto x = table.get_or<float_t>("x", table.get_or(1, 0.0));
      const auto y = table.get_or<float_t>("y", table.get_or(2, 0.0));
      o.set_velocity(algebra::vector2d{x, y});
    }

    algebra::vector2d get() const {
      return o.velocity();
    }
  };

  lua.new_usertype<velocityproxy>("VelocityProxy",
    "x", sol::property(&velocityproxy::x, &velocityproxy::set_x),
    "y", sol::property(&velocityproxy::y, &velocityproxy::set_y),
    "set", &velocityproxy::set,
    "get", &velocityproxy::get
  );

  struct metaobject {
    static sol::object index(framework::object& o, sol::stack_object key, sol::this_state state) {
      auto& store = o.kv();
      const auto& ptr = store.get(key.as<std::string>());
      return sol::make_object(state, std::ref(*ptr));
    }

    static void new_index(framework::object& o, sol::stack_object key, sol::stack_object value) {
      auto& store = o.kv();
      store.set(key.as<std::string>(), value);
    }
  };

  lua.new_usertype<memory::observable>(
    "Observable",
    sol::no_constructor,
    "value", sol::property(&memory::observable::value),
    "set", &memory::observable::set,
    "subscribe", &memory::observable::subscribe
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
    "hide", &framework::object::hide,
    "on_update", &framework::object::set_onupdate,
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
      [](framework::object& o, sol::table table) {
        auto x = .0f;
        auto y = .0f;

        if (table["x"].valid()) {
          x = table["x"].get<float_t>();
        } else if (table[1].valid()) {
          x = table[1].get<float_t>();
        }

        if (table["y"].valid()) {
          y = table["y"].get<float_t>();
        } else if (table[2].valid()) {
          y = table[2].get<float_t>();
        }

        o.set_placement(x, y);
      }
    ),
    "velocity", sol::property(
      [](framework::object& o) {
        return velocityproxy{o};
      },
      [](framework::object& o, const sol::object& v) {
        if (v.is<sol::table>()) {
          const auto table = v.as<sol::table>();
          const auto x = table.get_or<float_t>("x", table.get_or(1, 0));
          const auto y = table.get_or<float_t>("y", table.get_or(2, 0));
          o.set_velocity(algebra::vector2d{x, y});
          return;
        }

        if (v.is<algebra::vector2d>()) {
          o.set_velocity(v.as<algebra::vector2d>());
          return;
        }

        throw std::runtime_error("invalid value for velocity");
      }
    ),
    sol::meta_function::index, metaobject::index,
    sol::meta_function::new_index, metaobject::new_index
  );

  lua.new_usertype<framework::objectmanager>(
    "ObjectManager",
    sol::no_constructor,
    "create", &framework::objectmanager::create,
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

    playerwrapper(input::event::player player, const framework::statemanager& state_manager)
      : index(static_cast<uint8_t>(player)), e(state_manager) {}

    bool on(input::event::gamepad::button type) {
      return e.on(static_cast<uint8_t>(index), type);
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

  std::unordered_map<input::event::player, playerwrapper> player_mapping;

  lua.new_usertype<framework::statemanager>(
    "StateManager",
    sol::no_constructor,
    "collides", &statemanager::collides,
    "players", sol::property(&statemanager::players),
    "player", [&player_mapping](
      framework::statemanager& self,
      input::event::player player
    ) {
      const auto it = player_mapping.try_emplace(player, player, self).first;

      return it->second;
    }
  );

  lua.new_usertype<framework::scenemanager>(
    "SceneManager",
    sol::no_constructor,
    "set", &framework::scenemanager::set,
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
        auto sfn = [fn](float_t delta) mutable {
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
        auto sfn = [fn](float_t x, float_t y) mutable {
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
        auto sfn = [fn](float_t x, float_t y) mutable {
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
      }

      const auto end = SDL_GetPerformanceCounter();
      const auto elapsed = static_cast<double_t>(end - start) * 1000.0 / static_cast<double_t>(SDL_GetPerformanceFrequency());
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
    sol::constructors<geometry::point(), geometry::point(float_t, float_t)>(),
    "set", &geometry::point::set,
    "x", sol::property(&geometry::point::x, &geometry::point::set_x),
    "y", sol::property(&geometry::point::y, &geometry::point::set_y)
  );

  lua.new_usertype<geometry::size>(
    "Size",
    sol::constructors<geometry::size(), geometry::size(float_t, float_t), geometry::size(const geometry::size&)>(),
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
      sol::object object
    ) {
      if (object.is<int>())
        self.set<int>(key, object.as<int>());
      else if (object.is<double_t>())
        self.set<double_t>(key, object.as<double_t>());
      else if (object.is<bool>())
        self.set<bool>(key, object.as<bool>());
      else if (object.is<std::string>())
        self.set<std::string>(key, object.as<std::string>());
      else if (object.is<sol::table>()) {
        sol::table table = object.as<sol::table>();
        std::function<nlohmann::json(sol::table)> table2json = [&](sol::table table) {
          nlohmann::json json;
          auto isarray = true;
          for (const auto& pair : table) {
            if (!pair.first.is<int>()) {
              isarray = false;
              break;
            }
          }
          if (isarray) {
            json = nlohmann::json::array();
            for (auto i = 1uz, n = table.size(); i <= n; ++i) {
              sol::optional<sol::object> opt = table[i];
              if (opt) {
                sol::object o = opt.value();
                if (o.is<int>())
                  json.emplace_back(o.as<int>());
                else if (o.is<double_t>())
                  json.emplace_back(o.as<double_t>());
                else if (o.is<bool>())
                  json.emplace_back(o.as<bool>());
                else if (o.is<std::string>())
                  json.emplace_back(o.as<std::string>());
                else if (o.is<sol::table>())
                  json.emplace_back(table2json(o.as<sol::table>()));
                else
                  json.emplace_back(nullptr);
              } else {
                json.emplace_back(nullptr);
              }
            }
          } else {
            json = nlohmann::json::object();
            for (const auto& pair : table) {
              const auto& key = pair.first;
              const auto& value = pair.second;
              std::string k;
              if (key.is<std::string>())
                k = key.as<std::string>();
              else if (key.is<int>())
                k = std::to_string(key.as<int>());
              else
                continue;
              if (value.is<int>())
                json[k] = value.as<int>();
              else if (value.is<double_t>())
                json[k] = value.as<double_t>();
              else if (value.is<bool>())
                json[k] = value.as<bool>();
              else if (value.is<std::string>())
                json[k] = value.as<std::string>();
              else if (value.is<sol::table>())
                json[k] = table2json(value.as<sol::table>());
              else
                json[k] = nullptr;
            }
          }
          return json;
        };

        self.set<nlohmann::json>(key, table2json(table));
      } else {
        throw std::runtime_error("unsupported type for set");
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

      std::function<sol::object(const nlohmann::json&)> json2lua =
          [&](const nlohmann::json& json) -> sol::object {
        if (json.is_object()) {
          sol::table table = lua.create_table();
          for (auto it = json.begin(); it != json.end(); ++it)
            table[it.key()] = json2lua(it.value());
          return sol::make_object(lua, table);
        } else if (json.is_array()) {
          sol::table table = lua.create_table();
          int index = 1;
          for (const auto& item : json)
            table[index++] = json2lua(item);
          return sol::make_object(lua, table);
        } else if (json.is_number_integer())
          return sol::make_object(lua, json.get<int>());
        else if (json.is_number_float())
          return sol::make_object(lua, json.get<double_t>());
        else if (json.is_boolean())
          return sol::make_object(lua, json.get<bool>());
        else if (json.is_string())
          return sol::make_object(lua, json.get<std::string>());
        return sol::lua_nil;
      };
      return json2lua(j);
    }
  );

  lua.new_usertype<network::socket>(
    "Socket",
    sol::constructors<network::socket()>(),
    "connect", &network::socket::connect,
    "emit", [](
      network::socket& self,
      const std::string& event,
      const sol::table& data,
      sol::this_state state
    ) {
      sol::state_view lua(state);
      const auto j = _to_json(data);
      self.emit(event, j.dump());
    },
    "on", [](
      network::socket& self,
      const std::string& event,
      const sol::function& callback,
      sol::this_state state
    ) {
      sol::state_view lua(state);

      self.on(event, [callback, lua](const std::string& json) {
        const auto& j = nlohmann::json::parse(json);
        callback(_to_lua(j, lua));
      });
    },
    "rpc", [](
      network::socket& self,
      const std::string& method,
      const sol::table& arguments,
      const sol::function& callback,
      sol::this_state state
    ) {
      sol::state_view lua(state);
      const auto j = _to_json(arguments);

      self.rpc(method, j.dump(), [callback, lua](const std::string& response) {
        const auto& j = nlohmann::json::parse(response);
        callback(_to_lua(j, lua));
      });
    }
  );

  lua.new_usertype<graphics::color>(
    "Color",
    "color", sol::constructors<graphics::color(const std::string& )>(),

    "r", sol::property(&graphics::color::r, &graphics::color::set_r),
    "g", sol::property(&graphics::color::g, &graphics::color::set_g),
    "b", sol::property(&graphics::color::b, &graphics::color::set_b),
    "a", sol::property(&graphics::color::a, &graphics::color::set_a),

    sol::meta_function::equal_to,
    &graphics::color::operator==
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
    "clear", &framework::timermanager::clear
  );

  lua.new_usertype<algebra::vector2d>(
    "Vector2D",
    sol::constructors<algebra::vector2d(), algebra::vector2d(float_t, float_t)>(),

    "x", sol::property(&algebra::vector2d::x, &algebra::vector2d::set_x),
    "y", sol::property(&algebra::vector2d::y, &algebra::vector2d::set_y),
    "set", &algebra::vector2d::set,

    "magnitude", &algebra::vector2d::magnitude,
    "unit", &algebra::vector2d::unit,
    "dot", &algebra::vector2d::dot,

    sol::meta_function::addition, &algebra::vector2d::operator+,
    sol::meta_function::subtraction, &algebra::vector2d::operator-,

    "add_assign", &algebra::vector2d::operator+=,
    "sub_assign", &algebra::vector2d::operator-=,
    "mul_assign", &algebra::vector2d::operator*=,
    "div_assign", &algebra::vector2d::operator/=,

    sol::meta_function::equal_to, &algebra::vector2d::operator==,

    "zero", &algebra::vector2d::zero,
    "moving", &algebra::vector2d::moving,
    "right", &algebra::vector2d::right,
    "left", &algebra::vector2d::left
  );

  lua.new_usertype<graphics::label>(
    "Label",
    sol::no_constructor,
    sol::base_classes, sol::bases<graphics::widget>(),
    "font", sol::property(&graphics::label::set_font),
    "set", sol::overload(
      [](
        std::shared_ptr<graphics::label> self,
        float_t x,
        float_t y
      ) {
        self->set(x, y);
      },
      [](
        std::shared_ptr<graphics::label> self,
        const std::string& text,
        float_t x,
        float_t y
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
    // TODO "mode": sol::property(...)
    "pixels", sol::property(
      [](const graphics::canvas&) {
        return nullptr;
      },
      [](graphics::canvas& canvas, const char* data) {
        canvas.set_pixels(data);
      }
    )
  );

  std::println("Powered by Carimbo: https://carimbo.site");
  std::println("Version: {}", GIT_TAG);
  std::println("Built on: {}, {} UTC", __DATE__, __TIME__);

  const auto jit = lua["jit"];
  if (jit.valid()) {
    std::println("Runtime: {}", jit["version"].get<std::string>());
  } else {
    std::println("Runtime: {}", lua["_VERSION"].get<std::string>());
  }

  std::println("License: MIT");
  std::println("Author: Rodrigo Delduca https://rodrigodelduca.org");

  const auto buffer = storage::io::read("scripts/main.lua");
  std::string_view script{reinterpret_cast<const char*>(buffer.data()), buffer.size()};
  const auto scr = lua.safe_script(script, &sol::script_pass_on_error);
  if (!scr.valid()) [[unlikely]] {
    sol::error err = scr;
    throw std::runtime_error(err.what());
  }

  const auto setup = lua["setup"].get<sol::protected_function>();
  const auto sp = setup();
  if (!sp.valid()) [[unlikely]] {
    sol::error err = sp;
    throw std::runtime_error(err.what());
  }

  const auto engine = lua["engine"].get<std::shared_ptr<framework::engine>>();
  const auto loop = lua["loop"].get<sol::function>();
  engine->add_loopable(std::make_shared<lua_loopable>(lua, loop));

  const auto end = SDL_GetPerformanceCounter();
  const auto elapsed =
      (static_cast<double_t>(end - start) * 1000.0) / static_cast<double_t>(SDL_GetPerformanceFrequency());
  std::println("boot time {:.3f}ms", elapsed);

  engine->run();
}
