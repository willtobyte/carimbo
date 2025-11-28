#include "filesystem.hpp"

void filesystem::mount(const std::string_view filename, const std::string_view mountpoint) noexcept {
  [[maybe_unused]] const auto result = PHYSFS_mount(filename.data(), mountpoint.data(), true);
  [[maybe_unused]] const auto* const message = PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode());
  
  assert(result != 0 && 
    std::format("[PHYSFS_mount] failed to mount {} to {}. reason: {}",
      filename,
      mountpoint,
      message).c_str());
}
