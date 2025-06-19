#pragma once

#include "common.hpp"

namespace framework {
class engine;

class enginefactory final {
public:
  enginefactory() noexcept = default;
  virtual ~enginefactory() noexcept = default;

  enginefactory& with_title(const std::string& title) noexcept;
  enginefactory& with_width(int32_t width) noexcept;
  enginefactory& with_height(int32_t height) noexcept;
  enginefactory& with_scale(float_t scale) noexcept;
  enginefactory& with_gravity(float_t gravity) noexcept;
  enginefactory& with_fullscreen(bool fullscreen) noexcept;

  std::shared_ptr<engine> create() const;

private:
  std::string _title{"Untitled"};
  int32_t _width{800};
  int32_t _height{600};
  float_t _scale{1.0};
  float_t _gravity{9.8};
  bool _fullscreen{false};
};
}
