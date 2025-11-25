#include "filesystem.hpp"

void filesystem::mount(const std::string_view filename, const std::string_view mountpoint) {
  if (PHYSFS_mount(filename.data(), mountpoint.data(), true) == 0) [[unlikely]] {
    const auto* const message = PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode());

    throw std::runtime_error(
      std::format("[PHYSFS_mount] failed to mount {} to {}. reason: {}",
        filename,
        mountpoint,
        message)
    );
  }
}
