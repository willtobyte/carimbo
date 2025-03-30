#include "scenemanager.hpp"

using namespace framework;

scenemanager::scenemanager(std::shared_ptr<framework::resourcemanager> resourcemanager, std::shared_ptr<objectmanager> objectmanager) noexcept
    : _pixmappool(resourcemanager->pixmappool()), _objectmanager(std::move(objectmanager)) {}

void scenemanager::load(const std::string &name) noexcept {
  const auto buffer = storage::io::read("scenes/" + name + ".json");
  const auto j = nlohmann::json::parse(buffer);

  const auto background = _pixmappool->get(j["background"].get_ref<const std::string &>());
  geometry::size size{j.at("width").get<int32_t>(), j.at("height").get<int32_t>()};

  const auto &os = j.value("objects", nlohmann::json::array());
  const auto &i = os.items();
  std::unordered_map<std::string, std::shared_ptr<object>> objects(os.size());
  std::transform(
      i.begin(),
      i.end(),
      std::inserter(objects, objects.end()),
      [&](const auto &item) -> std::pair<std::string, std::shared_ptr<object>> {
        const auto &key = item.key();
        const auto &data = item.value();
        const auto &kind = data["kind"].template get_ref<const std::string &>();
        const auto &action = data["action"].template get_ref<const std::string &>();
        const auto x = data.value("x", 0);
        const auto y = data.value("y", 0);

        auto e = _objectmanager->create(kind);
        e->set_placement(x, y);
        e->set_action(action);
        return {key, e};
      }
  );

  _scene_mapping[name] = std::make_shared<scene>(background, objects, size);
}

void scenemanager::set(const std::string &name) noexcept {
  // if (auto it = _onleave_mapping.find(_current_scene); it != _onleave_mapping.end()) {
  //   it->second();
  // }

  _current_scene = name;

  // _background.reset();

  // const auto old = std::exchange(_objects, {});
  // for (const auto &pair : old) {
  //   _objectmanager->destroy(pair.second);
  // }

  // if (auto it = _onenter_mapping.find(name); it != _onenter_mapping.end()) {
  //   it->second();
  // }
}

void scenemanager::update(float_t delta) noexcept {
  UNUSED(delta);
  // if (auto it = _onloop_mapping.find(_current_scene); it != _onloop_mapping.end()) {
  //   it->second(delta);
  // }
}

void scenemanager::draw() const noexcept {
  // if (!_background) [[unlikely]] {
  //   return;
  // }

  // static geometry::point point{0, 0};
  // _background->draw({point, _size}, {point, _size});
  //
  _scene_mapping.at(_current_scene)->draw();
}

// std::shared_ptr<object> scenemanager::grab(const std::string &name, const std::string &key) const noexcept {
//   UNUSED(name);
//   UNUSED(key);
//   // if (const auto it = _objects.find(key); it != _objects.end()) {
//   //   return it->second;
//   // }

//   // return nullptr;
// }

void scenemanager::set_onenter(std::string name, std::function<void()> fn) {
  _onenter_mapping.insert_or_assign(std::move(name), std::move(fn));
}

void scenemanager::set_onloop(std::string name, std::function<void(float_t)> fn) {
  _onloop_mapping.insert_or_assign(std::move(name), std::move(fn));
}

void scenemanager::set_onleave(std::string name, std::function<void()> fn) {
  _onleave_mapping.insert_or_assign(std::move(name), std::move(fn));
}
