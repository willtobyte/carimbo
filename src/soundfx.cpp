#include "soundfx.hpp"

using namespace audio;

constexpr std::size_t buffer_4mb = 4 * 1024 * 1024;

static size_t ovPHYSFS_read(void *ptr, size_t size, size_t nmemb, void *source) {
  auto file = reinterpret_cast<PHYSFS_file *>(source);
  PHYSFS_sint64 result = PHYSFS_readBytes(file, ptr, static_cast<PHYSFS_uint32>(size) * static_cast<PHYSFS_uint32>(nmemb));
  return (result <= 0) ? 0 : static_cast<size_t>(result) / size;
}

static int ovPHYSFS_seek(void *source, ogg_int64_t offset, int whence) {
  auto file = reinterpret_cast<PHYSFS_file *>(source);
  PHYSFS_sint64 target = 0;

  switch (whence) {
    case SEEK_SET:
      target = offset;
      break;
    case SEEK_CUR:
      target = PHYSFS_tell(file);
      if (target < 0) return -1;
      target += offset;
      break;
    case SEEK_END:
      target = PHYSFS_fileLength(file);
      if (target < 0) return -1;
      target += offset;
      break;
    default:
      assert(false);
      return -1;
  }

  if (target < 0)
    return -1;

  return PHYSFS_seek(file, static_cast<PHYSFS_uint64>(target)) ? 0 : -1;
}

static int ovPHYSFS_close(void *source) {
  return PHYSFS_close(reinterpret_cast<PHYSFS_file *>(source)) ? 0 : -1;
}

static long ovPHYSFS_tell(void *source) {
  auto file = reinterpret_cast<PHYSFS_file *>(source);
  PHYSFS_sint64 pos = PHYSFS_tell(file);
  if (pos > std::numeric_limits<long>::max() || pos < std::numeric_limits<long>::min()) {
    return -1;
  }

  return static_cast<long>(pos);
}

static ov_callbacks PHYSFS_callbacks = {
    ovPHYSFS_read,
    ovPHYSFS_seek,
    ovPHYSFS_close,
    ovPHYSFS_tell
};

const char *ov_strerror(int code) {
  switch (code) {
  case OV_FALSE:
    return "A request for an ov_read() returned 0.";
  case OV_EOF:
    return "End of file reached";
  case OV_HOLE:
    return "Missing or corrupt data in the bitstream";
  case OV_EREAD:
    return "Read error while fetching compressed data for decode";
  case OV_EFAULT:
    return "Internal inconsistency in encode or decode state. Continuing is likely not possible.";
  case OV_EIMPL:
    return "Feature not implemented";
  case OV_EINVAL:
    return "Invalid argument was passed to a call";
  case OV_ENOTVORBIS:
    return "The given file was not recognized as Ogg Vorbis data.";
  case OV_EBADHEADER:
    return "The file is apparently an Ogg Vorbis stream, but contains a corrupted or undecipherable header.";
  case OV_EVERSION:
    return "The bitstream format revision of the given file is not supported.";
  case OV_EBADLINK:
    return "The given link exists in the Vorbis data stream, but is not decipherable due to garbage or corruption.";
  case OV_ENOSEEK:
    return "File is not seekable";
  default:
    return "Unknown";
  }
}

soundfx::soundfx(const std::string& filename) {
  std::unique_ptr<PHYSFS_File, decltype(&PHYSFS_close)> ptr{PHYSFS_openRead(filename.c_str()), PHYSFS_close};
  if (!ptr) [[unlikely]] {
    throw std::runtime_error(std::format("[PHYSFS_openRead] error while opening file: {}, error: {}", filename, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode())));
  }

  std::unique_ptr<OggVorbis_File, decltype(&ov_clear)> vf{new OggVorbis_File, ov_clear};
  if (ov_open_callbacks(ptr.get(), vf.get(), nullptr, 0, PHYSFS_callbacks) < 0) [[unlikely]] {
    throw std::runtime_error(std::format("[ov_open_callbacks] error while opening file: {}", filename));
  }

  [[maybe_unused]] auto *pointer = ptr.release();

  const auto info = ov_info(vf.get(), -1);
  if (!info) [[unlikely]] {
    throw std::runtime_error(std::format("[ov_info] failed to retrieve OggVorbis info file: {}", filename));
  }

  const auto format = (info->channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
  const auto frequency = static_cast<ALsizei>(info->rate);

  auto offset = 0L;

  std::vector<uint8_t> data;
  {
    const auto total_pcm = ov_pcm_total(vf.get(), -1);
    if (total_pcm < 0) [[unlikely]] {
      throw std::runtime_error(std::format("[ov_pcm_total] failed for file: {}", filename));
    }

    const auto bytes_per_sample = 2;
    const auto channels = static_cast<uint64_t>(info->channels);
    const auto total_bytes_64 = static_cast<uint64_t>(total_pcm) * channels * bytes_per_sample;
    if (total_bytes_64 > std::numeric_limits<size_t>::max()) [[unlikely]] {
      throw std::runtime_error(std::format("[decode] file too large: {}", filename));
    }

    const auto total_bytes = static_cast<size_t>(total_bytes_64);
    data.resize(total_bytes);
  }

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
  constexpr const auto bigendian = 0;
#else
  constexpr const auto bigendian = 1;
#endif

  size_t written = 0;
  do {
    const size_t available = data.size() - written;
    if (available == 0) {
      break;
    }

    const int to_read = (available > static_cast<size_t>(std::numeric_limits<int>::max()))
                        ? std::numeric_limits<int>::max()
                        : static_cast<int>(available);

    offset = ov_read(
      vf.get(),
      reinterpret_cast<char *>(data.data() + written),
      to_read,
      bigendian,
      2,
      1,
      nullptr
    );

    if (offset < 0) [[unlikely]] {
      throw std::runtime_error(std::format("[ov_read] error while reading file: {}, error: {}", filename, ov_strerror(static_cast<int32_t>(offset))));
    }

    if (offset == 0) {
      break;
    }

    written += static_cast<size_t>(offset);
  } while (true);

  data.resize(written);

  ALuint buffer;
  alGenBuffers(1, &buffer);
  alBufferData(buffer, format, data.data(), static_cast<ALsizei>(data.size()), frequency);

  alGenSources(1, &_source);
  alSourcei(_source, AL_BUFFER, static_cast<ALint>(buffer));
  alDeleteBuffers(1, &buffer);
}

soundfx::~soundfx() noexcept {
  alDeleteSources(1, &_source);
}

void soundfx::play(bool loop) const noexcept {
  alSourcei(_source, AL_LOOPING, loop);
  alSourcePlay(_source);
}

void soundfx::stop() const noexcept {
  alSourceStop(_source);
}
