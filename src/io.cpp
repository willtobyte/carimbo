#include "io.hpp"

using namespace storage;

std::vector<std::uint8_t> io::read(std::string_view filename) {
  auto ptr = std::unique_ptr<PHYSFS_File, decltype(&PHYSFS_close)>(PHYSFS_openRead(filename.data()), PHYSFS_close);

  PHYSFS_setBuffer(ptr.get(), 4 * 1024 * 1024);

  if (!ptr) [[unlikely]] {
    throw std::runtime_error(
      std::format("[PHYSFS_openRead] error while opening file: {}, error: {}",
        filename,
        PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode())));
  }

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
      std::format("[PHYSFS_readBytes] error reading file: {}, expected {} bytes but read {}, error: {}",
        filename,
        amount,
        result,
        PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode())));
  }

  return buffer;
}

std::vector<std::string> io::enumerate(std::string_view directory) {
  std::unique_ptr<char*[], void(*)(char**)> ptr(PHYSFS_enumerateFiles(directory.data()), [](char** list) { PHYSFS_freeList(list); });

  if (!ptr) [[unlikely]] {
    throw std::runtime_error(
      std::format("[PHYSFS_enumerateFiles] error while enumerating directory: {}, error: {}",
        directory,
        PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode())));
  }

  auto *const *array = ptr.get();

  auto view = std::views::iota(0)
    | std::views::take_while([&](size_t i) {
      return array[i] != nullptr;
    });

  std::vector<std::string> files;
  files.reserve(std::max<size_t>(0, static_cast<size_t>(std::ranges::distance(view))));
  for (const auto& i : view) {
    files.emplace_back(array[i]);
  }

  return files;
}
