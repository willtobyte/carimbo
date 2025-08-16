#include "scriptengine.hpp"

static std::array<uint64_t, 2> prng_state;

void seed(uint64_t value) {
  constexpr uint64_t mix = 0xdeadbeefcafebabeULL;
  if (value == 0) value = 1;
  prng_state[0] = value;
  prng_state[1] = value ^ mix;
}

uint64_t xorshift128plus() {
  const auto s1 = prng_state[0];
  const auto s0 = prng_state[1];
  const auto result = s0 + s1;

  prng_state[0] = s0;
  prng_state[1] = (s1 ^ (s1 << 23)) ^ s0 ^ ((s1 ^ (s1 << 23)) >> 18) ^ (s0 >> 5);

  return result;
}

double xorshift_random_double() {
  static constexpr const auto inv_max = 1.0 / static_cast<double>(std::numeric_limits<uint64_t>::max());

  return static_cast<double>(xorshift128plus()) * inv_max;
}

lua_Integer xorshift_random_int(lua_Integer low, lua_Integer high) {
  if (low > high) std::swap(low, high);

  const auto ulow = static_cast<uint64_t>(low);
  const auto range = static_cast<uint64_t>(high - low + 1);
  return static_cast<lua_Integer>(ulow + (xorshift128plus() % range));
}

[[noreturn]] void panic(sol::optional<std::string> maybe_message) {
  throw std::runtime_error(std::format("Lua panic: {}", maybe_message.value_or("unknown Lua error")));
}

sol::object searcher(sol::this_state state, const std::string& module) {
  sol::state_view lua{state};

  const auto filename = std::format("scripts/{}.lua", module);
  const auto buffer = storage::io::read(filename);
  std::string_view script(reinterpret_cast<const char *>(buffer.data()), buffer.size());

  const auto loader = lua.load(script, filename);
  if (!loader.valid()) [[unlikely]] {
    sol::error err = loader;
    throw std::runtime_error(err.what());
  }

  return sol::make_object(lua, loader.get<sol::protected_function>());
}

class lua_loopable final : public framework::loopable {
public:
  explicit lua_loopable(const sol::state_view& lua, sol::function function)
      : _L(lua.lua_state()),
        _function(std::move(function)) {}

  void loop(float_t delta) override {
    _function(delta);

    _frames++;
    const auto now = SDL_GetTicks();
    _elapsed += now - _start;
    _start = now;

    const auto memory = lua_gc(_L, LUA_GCCOUNT, 0);

    if (_elapsed >= 1000) [[unlikely]] {
      std::println("{:.1f} {}KB", static_cast<double_t>(_frames * _elapsed) * 0.001, memory);

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
  lua_State *_L;
  sol::function _function;
  uint64_t _frames{0};
  uint64_t _elapsed{0};
  uint64_t _start{SDL_GetTicks()};
};

auto _to_lua(const nlohmann::json& value, sol::state_view lua) -> sol::object {
  switch (value.type()) {
  case nlohmann::json::value_t::object: {
    auto t = lua.create_table();
    for (const auto& [k, v] : value.items()) {
      t[k] = _to_lua(v, lua);
    }

    return t;
  }

  case nlohmann::json::value_t::array: {
    auto t = lua.create_table();
    for (size_t i = 0; i < value.size(); ++i) {
      t[i + 1] = _to_lua(value[i], lua);
    }

    return t;
  }

  case nlohmann::json::value_t::string:
    return sol::make_object(lua, value.get<std::string>());
  case nlohmann::json::value_t::boolean:
    return sol::make_object(lua, value.get<bool>());
  case nlohmann::json::value_t::number_integer:
    return sol::make_object(lua, value.get<int64_t>());
  case nlohmann::json::value_t::number_unsigned:
    return sol::make_object(lua, value.get<uint64_t>());
  case nlohmann::json::value_t::number_float:
    return sol::make_object(lua, value.get<double_t>());
  default:
    return sol::lua_nil;
  }
}

auto _to_json(const sol::object& value) -> nlohmann::json {
  switch (value.get_type()) {
  case sol::type::table: {
    const auto lua_table = value.as<sol::table>();
    if (std::ranges::all_of(lua_table, [](const auto& pair) {
          return pair.first.get_type() == sol::type::number && pair.first.template as<size_t>() >= 1;
        }
      )
    ) {
      nlohmann::json j = nlohmann::json::array();
      for (const auto& pair : lua_table) {
        j.push_back(_to_json(pair.second));
      }

      return j;
    }

    nlohmann::json j = nlohmann::json::object();
    for (const auto& pair : lua_table) {
      j[pair.first.as<std::string>()] = _to_json(pair.second);
    }

    return j;
  }

  case sol::type::string:
    return value.as<std::string>();
  case sol::type::boolean:
    return value.as<bool>();
  case sol::type::number: {
    const auto number = value.as<double_t>();
    double_t intpart;
    const double_t frac = std::modf(number, &intpart);
    return std::abs(frac) < std::numeric_limits<double_t>::epsilon()
        ? static_cast<double_t>(static_cast<int64_t>(intpart))
        : number;
  }

  default:
    return nullptr;
  }
}

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

  lua["_"] = &text;

  lua["moment"] = &moment;

  lua["openurl"] = [](std::string_view url) {
    #ifdef EMSCRIPTEN
      const auto script = std::format(R"javascript(var a = document.createElement('a');
        a.href = "{}";
        a.target = "_blank";
        a.rel = "noopener noreferrer";
        document.body.appendChild(a);
        a.click();
        document.body.removeChild(a);)javascript",
      url);

      emscripten_run_script(script.c_str());
    #else
      SDL_OpenURL(url.data());
    #endif
  };

  lua["queryparam"] = [&lua](const std::string& key, const std::string& defval) -> sol::object {
    #ifdef EMSCRIPTEN
      const auto script = std::format(R"javascript(
        (function(){{
          var p = new URLSearchParams(window.location.search);
          var v = p.get("{}");
          return v !== null ? v : "{}";
        }})()
      )javascript", key, defval);

      const char* result = emscripten_run_script_string(script.c_str());
      if (!result || !*result) return sol::make_object(lua, defval);
      return sol::make_object(lua, std::string(result));
    #else
      UNUSED(key);
      return sol::make_object(lua, defval);
    #endif
  };

  const auto now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
  seed(static_cast<uint64_t>(now));

  lua["math"]["random"] = sol::overload(
    []() -> double {
      return xorshift_random_double();
    },
    [](lua_Integer upper) -> lua_Integer {
      return xorshift_random_int(1, upper);
    },
    [](lua_Integer lower, lua_Integer upper) -> lua_Integer {
      return xorshift_random_int(lower, upper);
    }
  );

  lua["math"]["randomseed"] = [](lua_Integer seed_value) {
    seed(static_cast<uint64_t>(seed_value));
  };

  steam::achievement achievement;

  lua.new_usertype<steam::achievement>(
    "Achievement",
    "unlock", &steam::achievement::unlock
  );

  lua["achievement"] = &achievement;

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
     "stop", &audio::soundfx::stop
  );

  lua.new_usertype<audio::soundmanager>(
    "SoundManager",
    sol::no_constructor,
    "play", [](audio::soundmanager& manager, const std::string& name, std::optional<bool> loop_opt) {
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
    "Anchor",
    "top", framework::anchor::top,
    "bottom", framework::anchor::bottom,
    "left", framework::anchor::left,
    "right", framework::anchor::right,
    "none", framework::anchor::none
  );

  lua.new_enum(
    "Reflection",
    "none", graphics::reflection::none,
    "horizontal", graphics::reflection::horizontal,
    "vertical", graphics::reflection::vertical,
    "both", graphics::reflection::both
  );

  lua.new_usertype<memory::kv>(
    "KeyValue",
    sol::no_constructor,
    "get", [](memory::kv& self, const std::string& key, sol::this_state state) { return self.get(key, state); },
    "set", [](memory::kv& self, const std::string& key, const sol::object& new_value, sol::this_state state) { self.set(key, new_value, state); },
    "subscribe", [](memory::kv& self, const std::string& key, const sol::function& callback, sol::this_state state) { self.subscribe(key, callback, state); }
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

  lua.new_usertype<framework::object>(
    "Object",
    sol::no_constructor,
    "id", sol::property(&framework::object::id),
    "x", sol::property(&framework::object::x, &framework::object::set_x),
    "y", sol::property(&framework::object::y, &framework::object::set_y),
    "alpha", sol::property(&framework::object::alpha, &framework::object::set_alpha),
    "scale", sol::property(&framework::object::scale, &framework::object::set_scale),
    "hide", &framework::object::hide,
    "on_update", &framework::object::set_onupdate,
    "on_animationfinished", &framework::object::set_onanimationfinished,
    "on_mail", &framework::object::set_onmail,
    "on_touch", &framework::object::set_ontouch,
    "on_hover", &framework::object::set_onhover,
    "on_unhover", &framework::object::set_onunhover,
    "on_collision", &framework::object::set_oncollision,
    "on_nthtick", &framework::object::set_onnthtick,
    "reflection", sol::property(
      [](framework::object& o) {
        return o.reflection();
      },
      [](framework::object& o, graphics::reflection r) {
        o.set_reflection(r);
      }
    ),
    "action", sol::property(
      [](framework::object& o) {
        return o.action();
      },
      [](framework::object& o, std::optional<std::string> v) {
        if (!v.has_value()) {
          o.unset_action();
          return;
        }

        o.set_action(*v);
      }
    ),
    "placement", sol::property(
      [](framework::object& o) {
        return o.placement();
      },
      [](framework::object& o, sol::table table) {
        float x = 0.0f;
        float y = 0.0f;

        if (table["x"].valid()) {
          x = table["x"].get<float>();
        } else if (table[1].valid()) {
          x = table[1].get<float>();
        }

        if (table["y"].valid()) {
          y = table["y"].get<float>();
        } else if (table[2].valid()) {
          y = table[2].get<float>();
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
          const auto x = table.get_or<float_t>("x", table.get_or(1, 0.0));
          const auto y = table.get_or<float_t>("y", table.get_or(2, 0.0));
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
    "kv", sol::property([](framework::object&o) -> memory::kv& { return o.kv(); })
  );

  lua.new_usertype<framework::objectmanager>(
    "ObjectManager",
    sol::no_constructor,
    "create", [](framework::objectmanager& self, const std::string& kind) {
      return self.create(kind, std::nullopt, true);
    },
    "clone", &framework::objectmanager::clone,
    "destroy", &framework::objectmanager::destroy
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
            filenames.push_back(value.as<std::string>());
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
    "player", [&player_mapping](framework::statemanager& self, input::event::player player) -> playerwrapper& {
      const auto [iterator, inserted] = player_mapping.try_emplace(player, player, self);

      return iterator->second;
    }
  );

  lua.new_usertype<framework::scenemanager>(
    "SceneManager",
    sol::no_constructor,
    "set", &framework::scenemanager::set,
    "destroy", &framework::scenemanager::destroy,
    "register", [&lua](framework::scenemanager& self, const std::string& name) {
      const auto start = SDL_GetPerformanceCounter();

      const auto scene = self.load(name);

      const auto buffer = storage::io::read(std::format("scenes/{}.lua", name));
      std::string_view script(reinterpret_cast<const char *>(buffer.data()), buffer.size());
      auto result = lua.safe_script(script, &sol::script_pass_on_error);
      if (!result.valid()) [[unlikely]] {
        sol::error err = result;
        throw std::runtime_error(err.what());
      }

      auto module = result.get<sol::table>();

      std::weak_ptr<framework::scene> ws = scene;

      module["get"] = [ws](sol::table, const std::string& name, framework::scenetype type) {
        if (auto scene = ws.lock()) [[likely]] {
          return scene->get(name, type);
        }

        throw std::runtime_error("scene expired");
      };

      if (auto fn = module["on_enter"].get<sol::protected_function>(); fn.valid()) {
        auto sfn = [fn]() mutable {
          sol::protected_function_result result = fn();
          if (!result.valid()) [[unlikely]] {
            sol::error err = result;
            throw std::runtime_error(err.what());
          }
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

      if (auto fn = module["on_leave"].get<sol::protected_function>(); fn.valid()) {
        auto sfn = [fn]() mutable {
          sol::protected_function_result result = fn();
          if (!result.valid()) [[unlikely]] {
            sol::error err = result;
            throw std::runtime_error(err.what());
          }
        };

        scene->set_onleave(std::move(sfn));
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
    "cursor", sol::property([](graphics::overlay& o) -> cursorproxy { return cursorproxy{o}; })
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
    "create", &framework::enginefactory::create
  );

  lua.new_usertype<geometry::point>(
    "Point",
    sol::constructors<geometry::point(), geometry::point(float_t, float_t)>(),
    "set", &geometry::point::set,
    "x", sol::property(&geometry::point::x, &geometry::point::set_x),
    "y", sol::property(&geometry::point::y, &geometry::point::set_y),
    sol::meta_function::to_string, [](const geometry::point& self) {
      return std::format("point({}, {})", self.x(), self.y());
    }
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
    "set", [](storage::cassette& self, const std::string& key, sol::object object) {
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
        std::function<nlohmann::json(sol::table)> table2json = [&](sol::table table) -> nlohmann::json {
          nlohmann::json temp;
          bool is_array = true;
          for (const auto& pair : table) {
            if (!pair.first.is<int>()) {
              is_array = false;
              break;
            }
          }
          if (is_array) {
            temp = nlohmann::json::array();
            for (size_t i = 1, n = table.size(); i <= n; ++i) {
              sol::optional<sol::object> opt = table[i];
              if (opt) {
                sol::object o = opt.value();
                if (o.is<int>())
                  temp.push_back(o.as<int>());
                else if (o.is<double_t>())
                  temp.push_back(o.as<double_t>());
                else if (o.is<bool>())
                  temp.push_back(o.as<bool>());
                else if (o.is<std::string>())
                  temp.push_back(o.as<std::string>());
                else if (o.is<sol::table>())
                  temp.push_back(table2json(o.as<sol::table>()));
                else
                  temp.push_back(nullptr);
              } else {
                temp.push_back(nullptr);
              }
            }
          } else {
            temp = nlohmann::json::object();
            for (const auto& pair : table) {
              const sol::object& key = pair.first;
              const sol::object& value = pair.second;
              std::string k;
              if (key.is<std::string>())
                k = key.as<std::string>();
              else if (key.is<int>())
                k = std::to_string(key.as<int>());
              else
                continue;
              if (value.is<int>())
                temp[k] = value.as<int>();
              else if (value.is<double_t>())
                temp[k] = value.as<double_t>();
              else if (value.is<bool>())
                temp[k] = value.as<bool>();
              else if (value.is<std::string>())
                temp[k] = value.as<std::string>();
              else if (value.is<sol::table>())
                temp[k] = table2json(value.as<sol::table>());
              else
                temp[k] = nullptr;
            }
          }
          return temp;
        };
        nlohmann::json j = table2json(table);
        self.set<nlohmann::json>(key, j);
      } else {
        throw std::runtime_error("unsupported type for set");
      }
    },
    "get", [](const storage::cassette& self, const std::string& key, sol::object default_value, sol::this_state state) -> sol::object {
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
    "emit", [](network::socket& self, const std::string& event, const sol::table& data, sol::this_state state) {
      sol::state_view lua(state);
      const auto j = _to_json(data);
      self.emit(event, j.dump());
    },
    "on", [](network::socket& self, const std::string& event, const sol::function& callback, sol::this_state state) {
      sol::state_view lua(state);

      self.on(event, [callback, lua](const std::string& json) {
        const auto& j = nlohmann::json::parse(json);
        callback(_to_lua(j, lua));
      });
    },
    "rpc", [](network::socket& self, const std::string& method, const sol::table& arguments, const sol::function& callback, sol::this_state state) {
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
      [](std::shared_ptr<graphics::label> self, float_t x, float_t y) {
        self->set(x, y);
      },
      [](std::shared_ptr<graphics::label> self, const std::string& text, float_t x, float_t y) {
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
    // TODO "mode": sol::property(...)
    "pixels", sol::property(
      [](const graphics::canvas&) {
        return nullptr;
      },
      [](graphics::canvas& canvas, const char* data) {
        canvas.set_pixels(reinterpret_cast<const uint32_t*>(data));
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
  std::string_view script(reinterpret_cast<const char *>(buffer.data()), buffer.size());
  const auto scr = lua.safe_script(script, &sol::script_pass_on_error);
  if (!scr.valid()) [[unlikely]] {
    sol::error err = scr;
    throw std::runtime_error(err.what());
  }

  const auto setup = lua["setup"].get<sol::protected_function>();
  const auto sr = setup();
  if (!sr.valid()) [[unlikely]] {
    sol::error err = sr;
    throw std::runtime_error(err.what());
  }

  const auto engine = lua["engine"].get<std::shared_ptr<framework::engine>>();
  const auto loop = lua["loop"].get<sol::function>();
  engine->add_loopable(std::make_shared<lua_loopable>(lua, loop));

  const auto end = SDL_GetPerformanceCounter();
  const auto elapsed = static_cast<double_t>(end - start) * 1000.0 / static_cast<double_t>(SDL_GetPerformanceFrequency());
  std::println("boot time {:.3f}ms", elapsed);

  engine->run();
}
