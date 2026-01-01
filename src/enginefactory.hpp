#pragma once

#include "common.hpp"

class engine;

class enginefactory final {
public:
  enginefactory() = default;
  ~enginefactory() = default;

  enginefactory& with_title(std::string_view title) noexcept;
  enginefactory& with_width(int width) noexcept;
  enginefactory& with_height(int height) noexcept;
  enginefactory& with_scale(float scale) noexcept;
  enginefactory& with_gravity(float gravity) noexcept;
  enginefactory& with_fullscreen(bool fullscreen) noexcept;
  enginefactory& with_sentry(std::string_view dsn) noexcept;
  enginefactory& with_ticks(int ticks) noexcept;

  std::shared_ptr<engine> create() const;

private:
  std::string _title{"Untitled"};
  int _width{800};
  int _height{600};
  float _scale{1.0};
  float _gravity{9.8f};
  bool _fullscreen{false};
  int _ticks{0};
};
