#include "io.hpp"

bool io::exists(std::string_view filename) noexcept {
  return PHYSFS_exists(filename.data());
}

std::vector<uint8_t> io::read(std::string_view filename) {
  const auto ptr = unwrap(
    std::unique_ptr<PHYSFS_File, PHYSFS_Deleter>(PHYSFS_openRead(filename.data())),
    std::format("error while opening file: {}", filename)
  );

  const auto length = PHYSFS_fileLength(ptr.get());
  [[maybe_unused]] const auto* const length_error = PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode());
  assert(length >= 0 &&
    std::format("[PHYSFS_fileLength] invalid file length, file: {}, error: {}",
      filename,
      length_error).c_str());

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

std::vector<std::string> io::enumerate(std::string_view directory) {
  std::unique_ptr<char*[], PHYSFS_Deleter> ptr(PHYSFS_enumerateFiles(directory.data()));
  assert(ptr != nullptr &&
    std::format("[PHYSFS_enumerateFiles] error while enumerating directory: {}", directory).c_str());

  auto* const *data = ptr.get();

  auto n = 0uz;
  while (data[n] != nullptr) ++n;

  std::vector<std::string> entries;
  entries.reserve(n);

  for (auto i = 0uz; i < n; ++i) {
    entries.emplace_back(data[i]);
  }

  return entries;
}
