#include "io.hpp"

using namespace storage;

std::vector<uint8_t> io::read(std::string_view filename) {
  const auto ptr = unwrap(
    std::unique_ptr<PHYSFS_File, PHYSFS_Deleter>(PHYSFS_openRead(filename.data())),
    std::format("error while opening file: {}", filename)
  );

  PHYSFS_setBuffer(ptr.get(), PHYSFS_BUFFER_SIZE);

  const auto length = PHYSFS_fileLength(ptr.get());
  if (length < 0) [[unlikely]] {
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

std::vector<std::string> io::enumerate(std::string_view directory) {
  const auto ptr = unwrap(
    std::unique_ptr<char*[], PHYSFS_Deleter>(PHYSFS_enumerateFiles(directory.data())),
    std::format("error while enumerating directory: {}", directory)
  );

  auto* const *array = ptr.get();

  auto n = 0uz;
  while (array[n] != nullptr) ++n;

  std::vector<std::string> names;
  names.reserve(n);

  for (auto i = 0uz; i < n; ++i) {
    names.emplace_back(array[i]);
  }

  return names;
}
