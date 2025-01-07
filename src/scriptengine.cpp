#include "scriptengine.hpp"

#include "color.hpp"
#include "common.hpp"
#include "enginefactory.hpp"
#include "entity.hpp"
#include "entitymanager.hpp"
#include "entityprops.hpp"
#include "event.hpp"
#include "io.hpp"
#include "kv.hpp"
#include "label.hpp"
#include "loopable.hpp"
#include "overlay.hpp"
#include "point.hpp"
#include "postalservice.hpp"
#include "socket.hpp"
#include "soundmanager.hpp"
#include "vector2d.hpp"
#include "widget.hpp"

sol::table require(sol::state &lua, const std::string &module) {
  const auto data = storage::io::read("scripts/" + module + ".lua");
  const auto script = std::string(data.begin(), data.end());
  const auto result = lua.script(script);

  return result.get<sol::table>();
}

class lua_loopable : public framework::loopable {
public:
  lua_loopable(const sol::state &lua, sol::function function)
      : _gc(lua["collectgarbage"].get<sol::function>()), _function(std::move(function)) {}

  virtual ~lua_loopable() = default;

  void loop(float_t delta) noexcept override {
    UNUSED(delta);

    _function();

    const auto mem = _gc("count").get<double>() / 1024.0;
    if (mem <= 8.0) [[likely]] {
      _gc("step", 8);
      return;
    }
    _gc("collect");
  }

private:
  sol::function _gc;
  sol::function _function;
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
    if (std::ranges::all_of(lua_table, [](const auto &pair) { return pair.first.get_type() == sol::type::number && pair.first.template as<size_t>() >= 1; })) {
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
    const auto num = value.as<double>();
    return std::trunc(num) == num ? static_cast<int64_t>(num) : num;
  }
  default:
    return nullptr;
  }
}

void framework::scriptengine::run() {
  sol::state lua;

  lua.open_libraries();

  lua["require"] = [&lua](const std::string &module) {
    return require(lua, module);
  };

  lua["JSON"] = lua.create_table_with(
      "parse",
      [](const std::string &json_str, sol::this_state state) {
        auto j = nlohmann::json::parse(json_str);
        sol::state_view lua(state);

        return _to_lua(j, lua);
      },
      "stringify",
      [](const sol::table &table) {
        nlohmann::json result;
        for (const auto &pair : table) {
          result[pair.first.as<std::string>()] = _to_json(pair.second);
        }
        return result.dump();
      }
  );

  lua.new_usertype<audio::soundmanager>(
      "SoundManager",
      "play", &audio::soundmanager::play,
      "stop", &audio::soundmanager::stop
  );

  lua.new_enum(
      "Controller",
      "up", input::joystickevent::up,
      "down", input::joystickevent::down,
      "left", input::joystickevent::left,
      "right", input::joystickevent::right,
      "triangle", input::joystickevent::triangle,
      "circle", input::joystickevent::circle,
      "cross", input::joystickevent::cross,
      "square", input::joystickevent::square
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

  struct reflectionproxy {
    entity &e;

    void set(graphics::reflection value) {
      e.set_reflection(value);
    }

    void unset() {
      e.set_reflection(graphics::reflection::none);
    }
  };

  lua.new_usertype<reflectionproxy>(
      "ReflectionProxy",
      "set", &reflectionproxy::set,
      "unset", &reflectionproxy::unset
  );

  struct actionproxy {
    entity &e;

    void set(const std::string &value) {
      e.set_action(value);
    }

    std::string get() const {
      return e.get_action();
    }

    void unset() {
      e.unset_action();
    }
  };

  lua.new_usertype<actionproxy>(
      "ActionProxy",
      "set", &actionproxy::set,
      "get", &actionproxy::get,
      "unset", &actionproxy::unset
  );

  struct placementproxy {
    entity &e;

    void set(int32_t x, int32_t y) noexcept {
      e.set_placement(x, y);
    }

    geometry::point get() const noexcept {
      return e.get_placement();
    }
  };

  lua.new_usertype<placementproxy>(
      "PlacementProxy",
      "set", &placementproxy::set,
      "get", &placementproxy::get
  );

  struct velocityproxy {
    entity &e;

    void set(int32_t x, int32_t y) {
      e.set_velocity({x, y});
    }

    algebra::vector2d get() {
      return e.velocity();
    }

    double_t get_x() const {
      return e.velocity().x();
    }

    void set_x(double_t x) {
      auto velocity = e.velocity();
      velocity.set_x(x);
      e.set_velocity(velocity);
    }

    double_t get_y() const {
      return e.velocity().y();
    }

    void set_y(double_t y) {
      auto velocity = e.velocity();
      velocity.set_y(y);
      e.set_velocity(velocity);
    }
  };

  lua.new_usertype<velocityproxy>(
      "VelocityProxy",
      "set", &velocityproxy::set,
      "get", &velocityproxy::get,
      "x", sol::property(&velocityproxy::get_x, &velocityproxy::set_x),
      "y", sol::property(&velocityproxy::get_y, &velocityproxy::set_y)
  );

  lua.new_usertype<memory::kv>(
      "KeyValue",
      "get", [](memory::kv &self, const std::string &key, sol::this_state state) { return self.get(key, state); },
      "set", [](memory::kv &self, const std::string &key, const sol::object &new_value, sol::this_state state) { self.set(key, new_value, state); },
      "subscribe", [](memory::kv &self, const std::string &key, const sol::function &callback, sol::this_state state) { self.subscribe(key, callback, state); }
  );

  lua.new_usertype<framework::entity>(
      "Entity",
      "id", sol::property(&framework::entity::id),
      "x", sol::property(&framework::entity::x),
      "y", sol::property(&framework::entity::y),
      "visible", sol::property(&framework::entity::visible),
      "size", sol::property(&framework::entity::size),
      "move", &framework::entity::move,
      "on_update", &framework::entity::set_onupdate,
      "on_animationfinished", &framework::entity::set_onanimationfinished,
      "on_mail", &framework::entity::set_onmail,
      "on_collision", &framework::entity::set_oncollision,
      "reflection", sol::property([](framework::entity &e) -> reflectionproxy { return reflectionproxy{e}; }),
      "action", sol::property([](framework::entity &e) -> actionproxy { return actionproxy{e}; }),
      "placement", sol::property([](framework::entity &e) -> placementproxy { return placementproxy{e}; }),
      "velocity", sol::property([](framework::entity &e) -> velocityproxy { return velocityproxy{e}; }),
      "kv", sol::property([](framework::entity &e) -> memory::kv & { return e.kv(); })
  );

  lua.new_usertype<framework::entitymanager>(
      "EntityManager",
      "spawn", &framework::entitymanager::spawn,
      "destroy", &framework::entitymanager::destroy
  );

  lua.new_usertype<framework::resourcemanager>(
      "ResourceManager",
      "flush", &framework::resourcemanager::flush,
      "prefetch", [](std::shared_ptr<resourcemanager> manager, sol::table table) {
        std::vector<std::string> filenames(table.size());
        std::ranges::transform(
            table,
            filenames.begin(),
            [](const auto &item) {
              return item.second.template as<std::string>();
            }
        );
        manager->prefetch(std::move(filenames));
      }
  );

  struct playerwrapper {
    int index;
    const framework::statemanager &e;

    playerwrapper(input::player player, const framework::statemanager &state_manager)
        : index(static_cast<int>(player)), e(state_manager) {}

    bool on(std::variant<input::joystickevent> type) {
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
      "one", input::player::one,
      "two", input::player::two
  );

  std::unordered_map<input::player, playerwrapper> _p;

  lua.new_usertype<framework::statemanager>(
      "StateManager",
      "player", [&_p](framework::statemanager &self, input::player player) -> playerwrapper & {
        auto [iterator, inserted] = _p.try_emplace(player, player, self);

        return iterator->second;
      }
  );

  lua.new_usertype<framework::scenemanager>(
      "SceneManager",
      "set", &framework::scenemanager::set
  );

  lua.new_enum(
      "WidgetType",
      "label", graphics::widgettype::label
  );

  lua.new_usertype<graphics::overlay>(
      "Overlay",
      "create", &graphics::overlay::create,
      "destroy", &graphics::overlay::destroy
  );

  lua.new_usertype<framework::engine>(
      "Engine",
      "add_loopable", &framework::engine::add_loopable,
      "entitymanager", &framework::engine::entitymanager,
      "fontfactory", &framework::engine::fontfactory,
      "overlay", &framework::engine::overlay,
      "resourcemanager", &framework::engine::resourcemanager,
      "soundmanager", &framework::engine::soundmanager,
      "statemanager", &framework::engine::statemanager,
      "scenemanager", &framework::engine::scenemanager,
      "run", &framework::engine::run
  );

  lua.new_usertype<graphics::font>(
      "Font",
      sol::constructors<graphics::font()>()
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
      sol::constructors<geometry::point(), geometry::point(int32_t, int32_t)>(),
      "set", &geometry::point::set,
      "x", sol::property(&geometry::point::x, &geometry::point::set_x),
      "y", sol::property(&geometry::point::y, &geometry::point::set_y),
      sol::meta_function::to_string, [](const geometry::point &p) {
        return "point(" + std::to_string(p.x()) + ", " + std::to_string(p.y()) + ")";
      }
  );

  lua.new_usertype<geometry::size>(
      "Size", sol::constructors<geometry::size(), geometry::size(int32_t, int32_t), geometry::size(const geometry::size &)>(),
      "width", sol::property(&geometry::size::width, &geometry::size::set_width),
      "height", sol::property(&geometry::size::height, &geometry::size::set_height)
  );

  lua.new_usertype<network::socket>(
      "Socket",
      sol::constructors<network::socket()>(),
      "connect", &network::socket::connect,
      "emit", [](network::socket &sio, const std::string &event, sol::table data, sol::this_state state) {
    sol::state_view lua(state);
    const auto j = _to_json(data);
    sio.emit(event, j.dump()); },
      "on", [](network::socket &sio, const std::string &event, sol::function callback, sol::this_state state) {
    sol::state_view lua(state);
    sio.on(event, [callback, lua](const std::string &data) {
      const auto j = nlohmann::json::parse(data);
      callback(_to_lua(j, lua));
    }); },
      "rpc", [](network::socket &sio, const std::string &method, sol::table arguments, sol::function callback, sol::this_state state) {
    sol::state_view lua(state);
    const auto args_json = _to_json(arguments);
    sio.rpc(method, args_json.dump(), [callback, lua](const std::string &response) {
      const auto j = nlohmann::json::parse(response);
      callback(_to_lua(j, lua));
    }); }
  );

  lua.new_usertype<graphics::color>(
      "Color",
      "color", sol::constructors<graphics::color(const std::string &)>(),

      "r", sol::property(&graphics::color::r, &graphics::color::set_r),
      "g", sol::property(&graphics::color::g, &graphics::color::set_g),
      "b", sol::property(&graphics::color::b, &graphics::color::set_b),
      "a", sol::property(&graphics::color::a, &graphics::color::set_a),

      sol::meta_function::equal_to, &graphics::color::operator==, // sol::meta_function::not_equal_to, &color::operator!=,

      sol::meta_function::to_string, [](const graphics::color &c) {
        return "color(" + std::to_string(c.r()) + ", " +
               std::to_string(c.g()) + ", " +
               std::to_string(c.b()) + ", " +
               std::to_string(c.a()) + ")";
      }
  );

  lua.new_enum(
      "KeyEvent",
      "up", input::keyevent::up,
      "left", input::keyevent::left,
      "down", input::keyevent::down,
      "right", input::keyevent::right,
      "space", input::keyevent::space
  );

  lua.new_usertype<framework::mail>(
      "Mail",
      sol::constructors<framework::mail(std::shared_ptr<entity>, std::shared_ptr<entity>, const std::string &)>()
  );

  lua.new_usertype<framework::postalservice>(
      "PostalService", sol::constructors<framework::postalservice()>(),
      "post", &framework::postalservice::post
  );

  lua.new_usertype<framework::timermanager>(
      "TimeManager", sol::constructors<framework::timermanager()>(),
      "set", &framework::timermanager::set,
      "singleshot", &framework::timermanager::singleshot,
      "clear", &framework::timermanager::clear
  );

  lua.new_usertype<algebra::vector2d>(
      "Vector2D", sol::constructors<algebra::vector2d(), algebra::vector2d(double_t, double_t)>(),

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
      sol::base_classes, sol::bases<graphics::widget>(),
      "font", sol::property(&graphics::label::set_font),
      "set", sol::overload([](graphics::label &self, const std::string &text) { self.set(text); }, [](graphics::label &self, const std::string &text, int32_t x, int32_t y) { self.set_with_placement(text, x, y); })
  );

  lua.new_usertype<graphics::widget>(
      "Widget",
      sol::no_constructor
  );

  lua.new_usertype<graphics::fontfactory>(
      "FontFactory",
      "get", &graphics::fontfactory::get
  );

  const auto script = storage::io::read("scripts/main.lua");

  lua.script(std::string_view(reinterpret_cast<const char *>(script.data()), script.size()));

  lua["setup"]();
  const auto loop = lua["loop"].get<sol::function>();
  const auto engine = lua["engine"].get<framework::engine *>();
  engine->add_loopable(std::make_shared<lua_loopable>(lua, std::move(loop)));
  lua["run"]();
}
