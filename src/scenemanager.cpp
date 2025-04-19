#include "scenemanager.hpp"

using namespace framework;

scenemanager::scenemanager(std::shared_ptr<framework::resourcemanager> resourcemanager, std::shared_ptr<objectmanager> objectmanager) noexcept
    : _resourcemanager(std::move(resourcemanager)), _objectmanager(std::move(objectmanager)) {
}

std::shared_ptr<scene> scenemanager::load(const std::string &name) noexcept {
  const auto buffer = storage::io::read("scenes/" + name + ".json");
  const auto j = nlohmann::json::parse(buffer);

  const auto pixmappool = _resourcemanager->pixmappool();
  const auto background = pixmappool->get(fmt::format("blobs/{}/background.png", name));
  geometry::size size{j.at("width").get<float_t>(), j.at("height").get<float_t>()};

  const auto &es = j.value("effects", nlohmann::json::array());
  auto view = es
      | std::views::transform([&name](auto& e) {
          return std::format("blobs/{}/{}.ogg", name, e.template get<std::string>());
        });

  std::vector<std::string> assets;
  assets.reserve(std::ranges::distance(view));
  std::ranges::copy(view, std::back_inserter(assets));

  _resourcemanager->prefetch(assets);

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
        std::optional<std::string> action =
            data.contains("action") && data["action"].is_string()
                ? std::optional<std::string>{std::string(data["action"].template get_ref<const std::string &>())}
                : std::nullopt;
        const auto x = data.value("x", 0);
        const auto y = data.value("y", 0);

        auto e = _objectmanager->create(name, kind, false);
        e->set_placement(x, y);
        if (action) {
          e->set_action(*action);
        }

        return {key, e};
      }
  );

  const auto s = std::make_shared<scene>(_objectmanager, background, objects, size);
  _scene_mapping[name] = s;
  return s;
}

void scenemanager::set(const std::string &name) noexcept {
  if (_scene) {
    _scene->on_leave();
    // _resourcemanager->flush();
  }

  _scene = _scene_mapping.at(name);

  _scene->on_enter();
}

std::shared_ptr<scene> scenemanager::get(const std::string &name) const noexcept {
  return _scene_mapping.at(name);
}

void scenemanager::destroy(const std::string &name) noexcept {
  const auto it = _scene_mapping.find(name);
  if (it == _scene_mapping.end()) {
    return;
  }

  if (_scene && _scene == it->second) {
    _scene->on_leave();
    _scene.reset();
  }

  _scene_mapping.erase(it);
  _resourcemanager->flush();
}

void scenemanager::on_touch(float_t x, float_t y) const noexcept {
  _scene->on_touch(x, y);
}

void scenemanager::on_motion(float_t x, float_t y) const noexcept {
  _scene->on_motion(x, y);
}

void scenemanager::update(float_t delta) noexcept {
  _scene->update(delta);
}

void scenemanager::draw() const noexcept {
  _scene->draw();
}
