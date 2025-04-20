#include "io.hpp"

using namespace storage;

std::vector<uint8_t> io::read(std::string_view filename) {
  auto ptr = std::unique_ptr<PHYSFS_File, decltype(&PHYSFS_close)>(PHYSFS_openRead(filename.data()), PHYSFS_close);

  if (!ptr) [[unlikely]] {
    panic("[PHYSFS_openRead] error while opening file: {}, error: {}", filename, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
  }

  const auto length = PHYSFS_fileLength(ptr.get());
  if (length <= 0) [[unlikely]] {
    panic("[PHYSFS_fileLength] invalid file length, file: {}, error: {}", filename, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
  }

  std::vector<uint8_t> buffer(length);
  const auto result = PHYSFS_readBytes(ptr.get(), buffer.data(), length);
  if (result != length) [[unlikely]] {
    panic("[PHYSFS_readBytes] error reading file: {}, expected {} bytes but read {}, error: {}", filename, length, result, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
  }

  return buffer;
}

std::vector<std::string> io::list(std::string_view directory) {
  auto deleter = [](char **list) { PHYSFS_freeList(list); };
  std::unique_ptr<char *[], decltype(deleter)> ptr(PHYSFS_enumerateFiles(directory.data()), deleter);

  if (!ptr) [[unlikely]] {
    panic("[PHYSFS_enumerateFiles] error while enumerating directory: {}, error: {}", directory, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
  }

  char *const *array = ptr.get();
  auto view = std::views::iota(0) | std::views::take_while([&](size_t i) { return array[i] != nullptr; });

  const auto count = std::ranges::distance(view);
  std::vector<std::string> files;
  files.reserve(count);
  for (size_t i : view) {
    files.emplace_back(array[i]);
  }

  return files;
}
