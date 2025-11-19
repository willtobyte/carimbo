#pragma once

#include "common.hpp"

namespace framework {
enum class scenekind : uint8_t {
  object = 0,
  effect,
  particle
};

class scene {
  public:
    explicit scene(std::string_view name, const nlohmann::json& j, std::shared_ptr<scenemanager> scenemanager);

    virtual ~scene() noexcept;

    virtual void update(float delta);

    virtual void draw() const;

    std::string_view name() const noexcept;

    std::variant<
      std::shared_ptr<object>,
      std::shared_ptr<audio::soundfx>,
      std::shared_ptr<graphics::particleprops>
    > get(std::string_view name, scenekind type) const;

    void on_enter() const;
    void on_leave() const;
    void on_text(std::string_view text) const;
    void on_touch(float x, float y) const;
    void on_key_press(int32_t code) const;
    void on_key_release(int32_t code) const;
    void on_motion(float x, float y) const;

    void set_onenter(std::function<void()>&& fn);
    void set_onloop(sol::protected_function fn);
    void set_oncamera(sol::protected_function fn);
    void set_onleave(std::function<void()>&& fn);
    void set_ontouch(sol::protected_function fn);
    void set_onkeypress(sol::protected_function fn);
    void set_onkeyrelease(sol::protected_function fn);
    void set_ontext(sol::protected_function fn);
    void set_onmotion(sol::protected_function fn);

  protected:
    void load();

    std::string _name;
    nlohmann::json _j;

    std::vector<std::pair<std::string, std::shared_ptr<object>>> _objects;
    std::vector<std::pair<std::string, std::shared_ptr<audio::soundfx>>> _effects;
    std::unordered_map<std::string, std::shared_ptr<graphics::particlebatch>> _particles;

    std::shared_ptr<scenemanager> _scenemanager;

    std::shared_ptr<objectmanager> _objectmanager;
    std::shared_ptr<graphics::particlesystem> _particlesystem;
    std::shared_ptr<resourcemanager> _resourcemanager;

    std::function<void()> _onenter;
    std::function<void(float)> _onloop;
    std::function<geometry::rectangle(float)> _oncamera;
    std::function<void()> _onleave;
    std::function<void(float, float)> _ontouch;
    std::function<void(int32_t)> _onkeypress;
    std::function<void(int32_t)> _onkeyrelease;
    std::function<void(std::string_view)> _ontext;
    std::function<void(float, float)> _onmotion;
};

class scenebackdrop : public scene {
  public:
    scenebackdrop(std::string_view name, const nlohmann::json& j, std::shared_ptr<scenemanager> scenemanager);

    virtual ~scenebackdrop() noexcept = default;

    virtual void draw() const noexcept override;

  private:
    std::shared_ptr<graphics::pixmap> _background;
};

class sceneblank : public scene {
  public:
    sceneblank(std::string_view name, const nlohmann::json& j, std::shared_ptr<scenemanager> scenemanager);

    virtual ~sceneblank() noexcept = default;
};

class scenetilemap : public scene {
  public:
    virtual ~scenetilemap() noexcept = default;

    virtual void update(float delta) override;

    virtual void draw() const override;
};
}
