#pragma once

#include "common.hpp"

namespace framework {
class world {
  public:
    void add(const std::shared_ptr<object>& object);
    void remove(const std::shared_ptr<object>& object);

    void update(float delta) noexcept;
    void draw() const noexcept;

  private:
    std::vector<std::weak_ptr<object>> _objects;
}
}
