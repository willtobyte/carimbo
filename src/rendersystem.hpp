#pragma once

#include "common.hpp"
#include "entity.hpp"
#include "system.hpp"
#include "componentmanager.hpp"

class rendersystem final : public system {
public:
  explicit rendersystem(componentmanager& components) noexcept 
    : _components(components) {}

  ~rendersystem() override = default;

  void update(float delta) noexcept override {
    auto& transforms = _components.get_array<transform>();
    auto& transparencies = _components.get_array<transparency>();

    for (size_t i = 0; i < entity_count(); ++i) {
      auto const e = entity_at(i);
      
      if (!transforms.has(e)) continue;
      
      auto& t = transforms.get(e);
      
      uint8_t alpha = 255;
      if (transparencies.has(e)) {
        alpha = transparencies.get(e).value;
      }

      _render(e, t, alpha, delta);
    }
  }

  void set_renderer(std::shared_ptr<renderer> r) noexcept {
    _renderer = std::move(r);
  }

private:
  void _render(entity e, const transform& t, uint8_t alpha, float delta) noexcept {
    // Implementação de renderização específica
    // Este é apenas um exemplo da estrutura
  }

  componentmanager& _components;
  std::shared_ptr<renderer> _renderer;
};