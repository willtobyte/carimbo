#include "scriptengine.hpp"

[[noreturn]] void panic(sol::optional<std::string> maybe_message) {
  throw std::runtime_error(fmt::format("Lua panic: {}", maybe_message.value_or("unknown Lua error")));
}

sol::object searcher(sol::this_state state, const std::string& module) {
  sol::state_view lua{state};

  const auto filename = fmt::format("scripts/{}.lua", module);
  const auto buffer = storage::io::read(filename);
  std::string_view script(reinterpret_cast<const char *>(buffer.data()), buffer.size());

  const auto loader = lua.load(script, filename);
  if (!loader.valid()) [[unlikely]] {
    sol::error err = loader;
    throw std::runtime_error(err.what());
  }

  return sol::make_object(lua, loader.get<sol::protected_function>());
}

class lua_loopable : public framework::loopable {
public:
  explicit lua_loopable(const sol::state_view &lua, sol::protected_function function)
      : _L(lua.lua_state()),
        _function(std::move(function)) {}

  void loop(float_t delta) override {
    const auto result = _function(delta);
    if (!result.valid()) [[unlikely]] {
      sol::error err = result;
      throw std::runtime_error(err.what());
    }

    const auto memory = lua_gc(_L, LUA_GCCOUNT, 0) / 1024.0;
    if (memory <= 8.0) [[likely]] {
      lua_gc(_L, LUA_GCSTEP, 8);
      return;
    }

    lua_gc(_L, LUA_GCCOLLECT, 0);
    lua_gc(_L, LUA_GCCOLLECT, 0);
  }

private:
  lua_State* _L;
  sol::protected_function _function;
};

auto _to_lua(const nlohmann::json &value, sol::state_view lua) -> sol::object {
  switch (value.type()) {
  case nlohmann::json::value_t::object: {
    auto t = lua.create_table();
    for (const auto &[k, v] : value.items()) {
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
    return sol::make_object(lua, value.get<double>());
  default:
    return sol::lua_nil;
  }
}

auto _to_json(const sol::object &value) -> nlohmann::json {
  switch (value.get_type()) {
  case sol::type::table: {
    const auto lua_table = value.as<sol::table>();
    if (std::ranges::all_of(lua_table, [](const auto &pair) {
          return pair.first.get_type() == sol::type::number && pair.first.template as<size_t>() >= 1;
        }
      )
    ) {
      nlohmann::json j = nlohmann::json::array();
      for (const auto &pair : lua_table) {
        j.push_back(_to_json(pair.second));
      }
      return j;
    }

    nlohmann::json j = nlohmann::json::object();
    for (const auto &pair : lua_table) {
      j[pair.first.as<std::string>()] = _to_json(pair.second);
    }
    return j;
  }

  case sol::type::string:
    return value.as<std::string>();
  case sol::type::boolean:
    return value.as<bool>();
  case sol::type::number: {
    const auto number = value.as<double>();
    double intpart;
    const double frac = std::modf(number, &intpart);
    return std::abs(frac) < std::numeric_limits<double>::epsilon()
        ? static_cast<double>(static_cast<int64_t>(intpart))
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

  const auto inject = fmt::format(R"lua(
    local list = package.searchers or package.loaders
    table.insert(list, searcher)
  )lua");

  lua.script(inject);

  lua["_"] = &text;

  lua["moment"] = &moment;

  lua["openurl"] = [](std::string_view url) {
    #ifdef EMSCRIPTEN
      const auto script = fmt::format(R"javascript(var a = document.createElement('a');
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

  lua["JSON"] = lua.create_table_with(
    "parse", [](const std::string &json, sol::this_state state) {
      const auto &j = nlohmann::json::parse(json);

      sol::state_view lua(state);

      return _to_lua(j, lua);
    },

    "stringify", [](const sol::table &table) {
      nlohmann::json result;
      for (const auto &pair : table) {
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
    "get", [](memory::kv &self, const std::string &key, sol::this_state state) { return self.get(key, state); },
    "set", [](memory::kv &self, const std::string &key, const sol::object &new_value, sol::this_state state) { self.set(key, new_value, state); },
    "subscribe", [](memory::kv &self, const std::string &key, const sol::function &callback, sol::this_state state) { self.subscribe(key, callback, state); }
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
    "move", &framework::object::move,
    "on_update", &framework::object::set_onupdate,
    "on_animationfinished", &framework::object::set_onanimationfinished,
    "on_mail", &framework::object::set_onmail,
    "on_touch", &framework::object::set_ontouch,
    "on_hover", &framework::object::set_onhover,
    "on_unhover", &framework::object::set_onunhover,
    "on_collision", &framework::object::set_oncollision,
    "on_nthtick", &framework::object::set_onnthtick,
    "reflection", sol::property(
      [](framework::object &o) {
        return o.reflection();
      },
      [](framework::object &o, graphics::reflection r) {
        o.set_reflection(r);
      }
    ),
    "action", sol::property(
      [](framework::object &o) {
        return o.action();
      },
      [](framework::object &o, std::optional<std::string> v) {
        if (v.has_value()) {
          o.set_action(*v);
          return;
        }

        o.unset_action();
      }
    ),
    "placement", sol::property(
      [](framework::object &o) {
        return o.placement();
      },
      [](framework::object &o, sol::table table) {
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
      [](framework::object &o) {
        return o.velocity();
      },
      [](framework::object &o, const sol::object &v) {
        if (v.is<algebra::vector2d>()) {
          o.set_velocity(v.as<algebra::vector2d>());
          return;
        }

        if (v.is<sol::table>()) {
          sol::table table = v.as<sol::table>();

          auto x = table.get_or<float_t>("x", table.get_or(1, 0.0));
          auto y = table.get_or<float_t>("y", table.get_or(2, 0.0));
          o.set_velocity(algebra::vector2d{x, y});
          return;
        }

        throw std::runtime_error("Invalid type for velocity. Must be Vector2D or table with x and y.");
      }
    ),
    "kv", sol::property([](framework::object &o) -> memory::kv & { return o.kv(); })
  );

  lua.new_usertype<framework::objectmanager>(
    "ObjectManager",
    sol::no_constructor,
    "create", [](framework::objectmanager &om, const std::string &kind) {
      return om.create(kind, std::nullopt, true);
    },
    "clone", &framework::objectmanager::clone,
    "destroy", &framework::objectmanager::destroy
  );

  lua.new_usertype<framework::resourcemanager>(
    "ResourceManager",
    sol::no_constructor,
    "flush", [&lua](framework::resourcemanager &self) {
      lua.collect_garbage();
      self.flush();
    },
    "prefetch", sol::overload(
        [](framework::resourcemanager &self) {
          self.prefetch();
        },

        [](framework::resourcemanager &self, sol::table table) {
          std::vector<std::string> filenames;
          filenames.reserve(table.size());
          for (auto &&[key, value] : table) {
            filenames.emplace_back(value.as<std::string>());
          }
          self.prefetch(std::move(filenames));
        },

        [](framework::resourcemanager &self, sol::variadic_args arguments) {
          std::vector<std::string> filenames;
          filenames.reserve(arguments.size());
          for (auto &&v : arguments) {
            filenames.push_back(v.as<std::string>());
          }
          self.prefetch(std::move(filenames));
        }
      )
  );

  struct playerwrapper {
    uint8_t index;
    const framework::statemanager &e;

    playerwrapper(input::event::player player, const framework::statemanager &state_manager)
      : index(static_cast<uint8_t>(player)), e(state_manager) {}

    bool on(std::variant<input::event::gamepad::button> type) {
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
    "player", [&player_mapping](framework::statemanager &self, input::event::player player) -> playerwrapper & {
      const auto [iterator, inserted] = player_mapping.try_emplace(player, player, self);

      return iterator->second;
    }
  );

  lua.new_usertype<framework::scenemanager>(
    "SceneManager",
    sol::no_constructor,
    "set", &framework::scenemanager::set,
    "destroy", &framework::scenemanager::destroy,
    "register", [&lua](framework::scenemanager &manager, const std::string &name) {
      const auto scene = manager.load(name);

      const auto buffer = storage::io::read(fmt::format("scenes/{}.lua", name));
      std::string_view script(reinterpret_cast<const char *>(buffer.data()), buffer.size());
      auto result = lua.safe_script(script, &sol::script_pass_on_error);
      if (!result.valid()) [[unlikely]] {
        sol::error err = result;
        throw std::runtime_error(err.what());
      }

      auto module = result.get<sol::table>();

      module["get"] = [scene](sol::table, const std::string &object, framework::scenetype type) {
        return scene->get(object, type);
      };

      if (auto fn = module["on_enter"].get<sol::protected_function>(); fn.valid()) {
        auto safe_fn = [fn]() mutable {
          sol::protected_function_result result = fn();
          if (!result.valid()) [[unlikely]] {
            sol::error err = result;
            throw std::runtime_error(err.what());
          }
        };

        scene->set_onenter(std::move(safe_fn));
      }

      if (auto fn = module["on_loop"].get<sol::protected_function>(); fn.valid()) {
        auto safe_fn = [fn](float_t delta) mutable {
          sol::protected_function_result result = fn(delta);
          if (!result.valid()) [[unlikely]] {
            sol::error err = result;
            throw std::runtime_error(err.what());
          }
        };

        scene->set_onloop(std::move(safe_fn));
      }

      if (auto fn = module["on_leave"].get<sol::protected_function>(); fn.valid()) {
        auto safe_fn = [fn]() mutable {
          sol::protected_function_result result = fn();
          if (!result.valid()) [[unlikely]] {
            sol::error err = result;
            throw std::runtime_error(err.what());
          }
        };

        scene->set_onleave(std::move(safe_fn));
      }

      if (auto fn = module["on_text"].get<sol::protected_function>(); fn.valid()) {
        auto safe_fn = [fn](const std::string &text) mutable {
          sol::protected_function_result result = fn(text);
          if (!result.valid()) [[unlikely]] {
            sol::error err = result;
            throw std::runtime_error(err.what());
          }
        };

        scene->set_ontext(std::move(safe_fn));
      }

      if (auto fn = module["on_touch"].get<sol::protected_function>(); fn.valid()) {
        auto safe_fn = [fn](float_t x, float_t y) mutable {
          sol::protected_function_result result = fn(x, y);
          if (!result.valid()) [[unlikely]] {
            sol::error err = result;
            throw std::runtime_error(err.what());
          }
        };

        scene->set_ontouch(std::move(safe_fn));
      }

      if (auto fn = module["on_keypress"].get<sol::protected_function>(); fn.valid()) {
        auto safe_fn = [fn](int32_t code) mutable {
          sol::protected_function_result result = fn(code);
          if (!result.valid()) [[unlikely]] {
            sol::error err = result;
            throw std::runtime_error(err.what());
          }
        };

        scene->set_onkeypress(std::move(safe_fn));
      }

      if (auto fn = module["on_keyrelease"].get<sol::protected_function>(); fn.valid()) {
        auto safe_fn = [fn](int32_t code) mutable {
          sol::protected_function_result result = fn(code);
          if (!result.valid()) [[unlikely]] {
            sol::error err = result;
            throw std::runtime_error(err.what());
          }
        };

        scene->set_onkeyrelease(std::move(safe_fn));
      }

      if (auto fn = module["on_motion"].get<sol::protected_function>(); fn.valid()) {
        auto safe_fn = [fn](float_t x, float_t y) mutable {
          sol::protected_function_result result = fn(x, y);
          if (!result.valid()) [[unlikely]] {
            sol::error err = result;
            throw std::runtime_error(err.what());
          }
        };

        scene->set_onmotion(std::move(safe_fn));
      }
    }
  );

  lua.new_enum(
    "WidgetType",
    "cursor", graphics::widgettype::cursor,
    "label", graphics::widgettype::label
  );

  struct cursorproxy {
    graphics::overlay &o;

    void set(const std::string &name) { o.set_cursor(name); }

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
    "cursor", sol::property([](graphics::overlay &o) -> cursorproxy { return cursorproxy{o}; })
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
    "effect", sol::property(&graphics::font::set_effect)
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
    sol::meta_function::to_string, [](const geometry::point &p) {
      return fmt::format("point({}, {})", p.x(), p.y());
    }
  );

  lua.new_usertype<geometry::size>(
    "Size",
    sol::constructors<geometry::size(), geometry::size(float_t, float_t), geometry::size(const geometry::size &)>(),
    "width", sol::property(&geometry::size::width, &geometry::size::set_width),
    "height", sol::property(&geometry::size::height, &geometry::size::set_height)
  );

  lua.new_usertype<storage::cassette>(
    "Cassette",
    sol::no_constructor,
    "clear", &storage::cassette::clear,
    "set", [](storage::cassette &c, const std::string &key, sol::object object) {
      if (object.is<int>())
        c.set<int>(key, object.as<int>());
      else if (object.is<double>())
        c.set<double>(key, object.as<double>());
      else if (object.is<bool>())
        c.set<bool>(key, object.as<bool>());
      else if (object.is<std::string>())
        c.set<std::string>(key, object.as<std::string>());
      else if (object.is<sol::table>()) {
        sol::table table = object.as<sol::table>();
        std::function<nlohmann::json(sol::table)> table2json = [&](sol::table tbl) -> nlohmann::json {
          nlohmann::json tmp;
          bool is_array = true;
          for (auto &pair : tbl) {
            if (!pair.first.is<int>()) {
              is_array = false;
              break;
            }
          }
          if (is_array) {
            tmp = nlohmann::json::array();
            for (size_t i = 1, n = tbl.size(); i <= n; ++i) {
              sol::optional<sol::object> opt = tbl[i];
              if (opt) {
                sol::object obj = opt.value();
                if (obj.is<int>())
                  tmp.push_back(obj.as<int>());
                else if (obj.is<double>())
                  tmp.push_back(obj.as<double>());
                else if (obj.is<bool>())
                  tmp.push_back(obj.as<bool>());
                else if (obj.is<std::string>())
                  tmp.push_back(obj.as<std::string>());
                else if (obj.is<sol::table>())
                  tmp.push_back(table2json(obj.as<sol::table>()));
                else
                  tmp.push_back(nullptr);
              } else {
                tmp.push_back(nullptr);
              }
            }
          } else {
            tmp = nlohmann::json::object();
            for (auto &pair : tbl) {
              sol::object keyo = pair.first;
              sol::object valueo = pair.second;
              std::string k;
              if (keyo.is<std::string>())
                k = keyo.as<std::string>();
              else if (keyo.is<int>())
                k = std::to_string(keyo.as<int>());
              else
                continue;
              if (valueo.is<int>())
                tmp[k] = valueo.as<int>();
              else if (valueo.is<double>())
                tmp[k] = valueo.as<double>();
              else if (valueo.is<bool>())
                tmp[k] = valueo.as<bool>();
              else if (valueo.is<std::string>())
                tmp[k] = valueo.as<std::string>();
              else if (valueo.is<sol::table>())
                tmp[k] = table2json(valueo.as<sol::table>());
              else
                tmp[k] = nullptr;
            }
          }
          return tmp;
        };
        nlohmann::json j = table2json(table);
        c.set<nlohmann::json>(key, j);
      } else {
        throw std::runtime_error("unsupported type for set");
      }
    },
    "get", [](const storage::cassette &c, const std::string &key, sol::object default_value, sol::this_state ts) -> sol::object {
      sol::state_view lua(ts);
      const nlohmann::json j = c.get<nlohmann::json>(key, _to_json(default_value));

      std::function<sol::object(const nlohmann::json &)> json2lua =
          [&](const nlohmann::json &js) -> sol::object {
        if (js.is_object()) {
          sol::table tbl = lua.create_table();
          for (auto it = js.begin(); it != js.end(); ++it)
            tbl[it.key()] = json2lua(it.value());
          return sol::make_object(lua, tbl);
        } else if (js.is_array()) {
          sol::table tbl = lua.create_table();
          int index = 1;
          for (const auto &item : js)
            tbl[index++] = json2lua(item);
          return sol::make_object(lua, tbl);
        } else if (js.is_number_integer())
          return sol::make_object(lua, js.get<int>());
        else if (js.is_number_float())
          return sol::make_object(lua, js.get<double>());
        else if (js.is_boolean())
          return sol::make_object(lua, js.get<bool>());
        else if (js.is_string())
          return sol::make_object(lua, js.get<std::string>());
        return sol::lua_nil;
      };
      return json2lua(j);
    }
  );

  lua.new_usertype<network::socket>(
    "Socket",
    sol::constructors<network::socket()>(),
    "connect", &network::socket::connect,
    "emit", [](network::socket &sio, const std::string &event, sol::table data, sol::this_state state) {
      sol::state_view lua(state);
      const auto j = _to_json(data);
      sio.emit(event, j.dump());
    },
    "on", [](network::socket &sio, const std::string &event, sol::function callback, sol::this_state state) {
      sol::state_view lua(state);
      sio.on(event, [callback, lua](const std::string &json) {
        const auto &j = nlohmann::json::parse(json);

        callback(_to_lua(j, lua));
      });
    },
    "rpc", [](network::socket &sio, const std::string &method, sol::table arguments, sol::function callback, sol::this_state state) {
      sol::state_view lua(state);
      const auto args_json = _to_json(arguments);
      sio.rpc(
        method, args_json.dump(),
        [callback, lua](const std::string &response) {
          const auto &j = nlohmann::json::parse(response);

          callback(_to_lua(j, lua));
        }
      );
    }
  );

  lua.new_usertype<graphics::color>(
    "Color",
    "color", sol::constructors<graphics::color(const std::string &)>(),

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
      const std::string &
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
    sol::constructors<graphics::label()>(),
    sol::base_classes,
    sol::bases<graphics::widget>(),
    "font", sol::property(&graphics::label::set_font),
    "set", &graphics::label::set,
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
    "pixels", sol::property(
      [](graphics::canvas &) -> sol::object {
        return sol::lua_nil;
      },
      [](graphics::canvas &canvas, sol::table table) {
        const auto n = table.size();
        static std::vector<uint32_t> pixels(n);
        std::ranges::transform(
          std::views::iota(1u, n + 1u), pixels.begin(),
          [&table](auto index) {
            return table[index].template get<uint32_t>();
          }
        );

        canvas.set_pixels(pixels);
      }
    )
  );

  const auto jit = lua["jit"];
  if (jit.valid()) {
    fmt::println("{}", jit["version"].get<std::string>());
  }

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
  const auto loop = lua["loop"].get<sol::protected_function>();
  engine->add_loopable(std::make_shared<lua_loopable>(lua, loop));

  const auto end = SDL_GetPerformanceCounter();
  const auto elapsed = static_cast<double>(end - start) * 1000.0 / static_cast<double>(SDL_GetPerformanceFrequency());
  fmt::println("boot time {:.3f}ms", elapsed);

  engine->run();
}
