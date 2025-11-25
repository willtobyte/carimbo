#pragma once

#include "common.hpp"

#include "entity.hpp"

class entitymanager final {
public:
  entitymanager() noexcept;

  [[nodiscard]] entity create() noexcept;

  void destroy(const entity id) noexcept;

  void set_signature(const entity id, const ::signature sig) noexcept;

  [[nodiscard]] signature signature(const entity id) const noexcept;

private:
  std::array<::signature, entity_n> signatures;
  std::vector<entity> entities;
};
