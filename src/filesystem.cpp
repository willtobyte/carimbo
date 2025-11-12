#include "filesystem.hpp"

using namespace storage;

void filesystem::mount(const std::string_view filename, const std::string_view mountpoint) {
  if (PHYSFS_mount(filename.data(), mountpoint.data(), true) == 0) [[unlikely]] {
    const auto error_code = PHYSFS_getLastErrorCode();
    const auto* const error_msg = PHYSFS_getErrorByCode(error_code);

    throw std::runtime_error(
      std::format("[PHYSFS_mount] failed to mount {} to {}. reason: {}",
        filename,
        mountpoint,
        error_msg));
  }
}
