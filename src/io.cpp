#include "io.hpp"

bool io::exists(std::string_view filename) noexcept {
  return PHYSFS_exists(filename.data());
}

std::vector<uint8_t> io::read(std::string_view filename) {
  const auto ptr = unwrap(
    std::unique_ptr<PHYSFS_File, PHYSFS_Deleter>(PHYSFS_openRead(filename.data())),
    std::format("error while opening file: {}", filename)
  );

  PHYSFS_setBuffer(ptr.get(), PHYSFS_BUFFER_SIZE);

  const auto length = PHYSFS_fileLength(ptr.get());
  [[maybe_unused]] const auto* const error_msg = PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode());
  assert(length >= 0 &&
    std::format("[PHYSFS_fileLength] invalid file length, file: {}, error: {}",
      filename,
      error_msg).c_str());

  const auto amount = static_cast<std::size_t>(length);
  std::vector<uint8_t> buffer(amount);
  const auto result = PHYSFS_readBytes(ptr.get(), buffer.data(), amount);
  [[maybe_unused]] const auto* const read_error = PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode());
  assert(result == length &&
    std::format("[PHYSFS_readBytes] error reading file: {}, expected {} bytes however read {}, error: {}",
      filename,
      amount,
      result,
      read_error).c_str());

  return buffer;
}

boost::container::small_vector<std::string, 32> io::enumerate(std::string_view directory) noexcept {
  std::unique_ptr<char*[], PHYSFS_Deleter> ptr(PHYSFS_enumerateFiles(directory.data()));
  assert(ptr != nullptr &&
    std::format("error while enumerating directory: {}", directory).c_str());

  auto* const *array = ptr.get();

  auto n = 0uz;
  while (array[n] != nullptr) ++n;

  boost::container::small_vector<std::string, 32> names;

  for (auto i = 0uz; i < n; ++i) {
    names.emplace_back(array[i]);
  }

  return names;
}
