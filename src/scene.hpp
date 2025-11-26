#pragma once

#include "common.hpp"

struct alignas(8) transform final {
	vec2 position;
	double angle;
	float scale;

	[[nodiscard]] constexpr bool operator==(const transform&) const noexcept = default;
};

struct alignas(4) tint final {
  uint8_t r{0};
  uint8_t g{0};
  uint8_t b{0};
  uint8_t a{255};

  [[nodiscard]] constexpr bool operator==(const tint&) const noexcept = default;
};

struct alignas(16) timeline final {
  static constexpr auto capacity = 16;

  std::array<char, 64> action;
  std::array<char, 64> next;
  std::array<quad, capacity> frames;
  std::array<uint16_t, capacity> durations;
  uint16_t count{0};
  uint16_t current{0};
  uint64_t tick{0};

  [[nodiscard]] constexpr bool operator==(const timeline&) const noexcept = default;
};

struct alignas(8) sprite final {
  uint64_t id;
  reflection reflection;

  [[nodiscard]] constexpr bool operator==(const sprite&) const noexcept = default;
};

class scene {
public:
  scene(std::string_view name, const nlohmann::json& json, std::shared_ptr<scenemanager> scenemanager);

  void update(float delta) noexcept;

  void draw() const noexcept;

private:
  entt::registry _registry;

  uint64_t _sprite_counter;

  std::unordered_map<size_t, std::shared_ptr<pixmap>> _spritesheets;

  std::shared_ptr<pixmap> _background;
};



// enum class scenekind : uint8_t {
//   object = 0,
//   effect,
//   particle
// };

// class scene {
// public:
//   explicit scene(std::string_view name, const nlohmann::json& j, std::shared_ptr<scenemanager> scenemanager);

//   virtual ~scene() noexcept;

//   virtual void update(float delta);

//   virtual void draw() const;

//   std::string_view name() const noexcept;

//   std::variant<
//     std::shared_ptr<object>,
//     std::shared_ptr<soundfx>,
//     std::shared_ptr<particleprops>
//   > get(std::string_view name, scenekind type) const;

//   void on_enter() const;
//   void on_leave() const;
//   void on_text(std::string_view text) const;
//   void on_touch(float x, float y) const;
//   void on_key_press(int32_t code) const;
//   void on_key_release(int32_t code) const;
//   void on_motion(float x, float y) const;

//   void set_onenter(std::function<void()>&& fn);
//   void set_onloop(sol::protected_function fn);
//   void set_oncamera(sol::protected_function fn);
//   void set_onleave(std::function<void()>&& fn);
//   void set_ontouch(sol::protected_function fn);
//   void set_onkeypress(sol::protected_function fn);
//   void set_onkeyrelease(sol::protected_function fn);
//   void set_ontext(sol::protected_function fn);
//   void set_onmotion(sol::protected_function fn);

// protected:
//   void load();

//   std::string _name;
//   nlohmann::json _j;

//   quad _camera;
//   std::vector<std::pair<std::string, std::shared_ptr<object>>> _objects;
//   std::vector<std::pair<std::string, std::shared_ptr<soundfx>>> _effects;
//   std::unordered_map<std::string, std::shared_ptr<particlebatch>> _particles;

//   std::shared_ptr<scenemanager> _scenemanager;

//   std::shared_ptr<objectmanager> _objectmanager;
//   std::shared_ptr<particlesystem> _particlesystem;
//   std::shared_ptr<resourcemanager> _resourcemanager;

//   std::function<void()> _onenter;
//   std::function<void(float)> _onloop;
//   std::function<quad(float)> _oncamera;
//   std::function<void()> _onleave;
//   std::function<void(float, float)> _ontouch;
//   std::function<void(int32_t)> _onkeypress;
//   std::function<void(int32_t)> _onkeyrelease;
//   std::function<void(std::string_view)> _ontext;
//   std::function<void(float, float)> _onmotion;
// };

// class scenebackdrop : public scene {
// public:
//   scenebackdrop(std::string_view name, const nlohmann::json& j, std::shared_ptr<scenemanager> scenemanager);

//   virtual ~scenebackdrop() noexcept = default;

//   virtual void draw() const noexcept override;

// private:
//   std::shared_ptr<pixmap> _background;
// };
