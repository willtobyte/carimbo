#include "filesystem.hpp"

using namespace storage;

void filesystem::mount(const std::string& filename, const std::string& mountpoint) {
  if (PHYSFS_mount(filename.c_str(), mountpoint.c_str(), true) == 0) [[unlikely]] {
    throw std::runtime_error(
      std::format("[PHYSFS_mount] failed to mount {} to {}. reason: {}",
        filename,
        mountpoint,
        PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode())));
  }
}
