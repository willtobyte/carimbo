#include "tilemap.hpp"

using namespace framework;

tilemap::tilemap(std::shared_ptr<resourcemanager> resourcemanager, const std::string &name) noexcept
  : _resourcemanager(std::move(resourcemanager)) {
  UNUSED(name);
}
