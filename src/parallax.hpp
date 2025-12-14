#pragma once

#include "common.hpp"

struct alignas(8) repeat final {
  bool x{true};
  bool y{false};

  friend void from_json(unmarshal::value json, repeat& out);
};

struct alignas(32) parallax_layer final {
  std::shared_ptr<pixmap> texture;
  vec2 speed{1.0f, 1.0f};
  vec2 offset{0.0f, 0.0f};
  int8_t z_order{0};
  repeat repeat{};

  friend void from_json(unmarshal::value json, parallax_layer& out);
};

class parallax final {
public:
  parallax() = default;

  parallax(
      std::string_view scene,
      unmarshal::document& document,
      std::shared_ptr<resourcemanager> resourcemanager
  );

  ~parallax() = default;

  void set_camera(vec2 position) noexcept;

  void update(float delta) noexcept;

  void draw_back() const noexcept;

  void draw_front() const noexcept;

private:
  void draw_layer(const parallax_layer& layer) const noexcept;

  std::vector<parallax_layer> _back_layers;
  std::vector<parallax_layer> _front_layers;
  vec2 _camera{};
  std::shared_ptr<renderer> _renderer;
  int _width{};
  int _height{};
};
