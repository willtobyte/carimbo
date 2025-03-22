#include "io.hpp"

using namespace storage;

std::vector<uint8_t> io::read(const std::string &filename) {
  auto ptr = std::unique_ptr<PHYSFS_File, decltype(&PHYSFS_close)>(PHYSFS_openRead(filename.c_str()), PHYSFS_close);

  if (!ptr) [[unlikely]] {
    throw std::runtime_error(fmt::format("[PHYSFS_openRead] error while opening file: {}, error: {}", filename, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode())));
  }

  const auto length = PHYSFS_fileLength(ptr.get());
  if (length <= 0) [[unlikely]] {
    throw std::runtime_error(fmt::format("[PHYSFS_fileLength] invalid file length, file: {}, error: {}", filename, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode())));
  }

  std::vector<uint8_t> buffer(length);
  const auto result = PHYSFS_readBytes(ptr.get(), buffer.data(), length);
  if (result != length) [[unlikely]] {
    throw std::runtime_error(fmt::format("[PHYSFS_readBytes] error reading file: {}, expected {} bytes but read {}, error: {}", filename, length, result, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode())));
  }

  return buffer;
}
