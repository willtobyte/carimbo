#include "filesystem.hpp"

using namespace storage;

void filesystem::mount(std::string_view filename, std::string_view mountpoint) {
  // Garantir null-terminated strings para PHYSFS
  std::string filename_str{filename};
  std::string mountpoint_str{mountpoint};
  
  if (PHYSFS_mount(filename_str.c_str(), mountpoint_str.c_str(), true) == 0) [[unlikely]] {
    throw std::runtime_error(
      std::format("[PHYSFS_mount] failed to mount {} to {}. reason: {}",
        filename,
        mountpoint,
        PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode())));
  }
}
