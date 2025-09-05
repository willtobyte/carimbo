#include "io.hpp"

using namespace storage;

std::vector<std::uint8_t> io::read(const std::string& filename) {
  const auto ptr = std::unique_ptr<PHYSFS_File, decltype(&PHYSFS_close)>(PHYSFS_openRead(filename.c_str()), PHYSFS_close);

  if (!ptr) [[unlikely]] {
    throw std::runtime_error(
      std::format("[PHYSFS_openRead] error while opening file: {}, error: {}",
        filename,
        PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode())));
  }

  PHYSFS_setBuffer(ptr.get(), 4 * 1024 * 1024);

  const auto length = PHYSFS_fileLength(ptr.get());
  if (length <= 0) [[unlikely]] {
    throw std::runtime_error(
      std::format("[PHYSFS_fileLength] invalid file length, file: {}, error: {}",
        filename,
        PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode())));
  }

  const auto amount = static_cast<std::size_t>(length);
  std::vector<uint8_t> buffer(amount);
  const auto result = PHYSFS_readBytes(ptr.get(), buffer.data(), amount);
  if (result != length) [[unlikely]] {
    throw std::runtime_error(
      std::format("[PHYSFS_readBytes] error reading file: {}, expected {} bytes however read {}, error: {}",
        filename,
        amount,
        result,
        PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode())));
  }

  return buffer;
}

std::vector<std::string> io::enumerate(const std::string& directory) {
  const std::unique_ptr<char*[], void(*)(char**)> ptr(PHYSFS_enumerateFiles(directory.c_str()), [](char** list) { PHYSFS_freeList(list); });

  if (!ptr) [[unlikely]] {
    throw std::runtime_error(
      std::format("[PHYSFS_enumerateFiles] error while enumerating directory: {}, error: {}",
        directory,
        PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode())));
  }

  auto *const *array = ptr.get();

  auto n = 0uz;
  while (array[n] != nullptr) ++n;

  std::vector<std::string> names;
  names.reserve(n);

  for (auto i = 0uz; i < n; ++i) {
    names.emplace_back(array[i]);
  }

  return names;
}
