#include "soundfx.hpp"

using namespace audio;

static size_t ovPHYSFS_read(void *ptr, size_t size, size_t nmemb, void *source) {
  auto file = reinterpret_cast<PHYSFS_file *>(source);
  PHYSFS_sint64 result = PHYSFS_readBytes(file, ptr, static_cast<PHYSFS_uint32>(size) * static_cast<PHYSFS_uint32>(nmemb));
  return (result <= 0) ? 0 : static_cast<size_t>(result) / size;
}

static int ovPHYSFS_seek(void *source, ogg_int64_t offset, int whence) {
  auto file = reinterpret_cast<PHYSFS_file *>(source);
  switch (whence) {
  case SEEK_SET:
    return PHYSFS_seek(file, static_cast<PHYSFS_uint64>(offset)) ? 0 : -1;
  case SEEK_CUR:
    return PHYSFS_seek(file, PHYSFS_tell(file) + offset) ? 0 : -1;
  case SEEK_END:
    return PHYSFS_seek(file, PHYSFS_fileLength(file) + offset) ? 0 : -1;
  default:
    assert(false);
    return -1;
  }
}

static int ovPHYSFS_close(void *source) {
  return PHYSFS_close(reinterpret_cast<PHYSFS_file *>(source)) ? 0 : -1;
}

static long ovPHYSFS_tell(void *source) {
  return static_cast<long>(PHYSFS_tell(reinterpret_cast<PHYSFS_file *>(source)));
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

soundfx::soundfx(const std::string &filename) {
  std::unique_ptr<PHYSFS_File, decltype(&PHYSFS_close)> fp{PHYSFS_openRead(filename.c_str()), PHYSFS_close};
  if (!fp) [[unlikely]] {
    throw std::runtime_error(fmt::format("[PHYSFS_openRead] error while opening file: {}, error: {}", filename, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode())));
  }

  std::unique_ptr<OggVorbis_File, decltype(&ov_clear)> vf{new OggVorbis_File, ov_clear};
  if (ov_open_callbacks(fp.get(), vf.get(), nullptr, 0, PHYSFS_callbacks) < 0) [[unlikely]] {
    throw std::runtime_error("[ov_open_callbacks] error while opening file");
  }

  [[maybe_unused]] auto *pointer = fp.release();

  const auto info = ov_info(vf.get(), -1);
  if (!info) [[unlikely]] {
    throw std::runtime_error("[ov_info] failed to retrieve OggVorbis info");
  }

  const auto format = (info->channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
  const auto frequency = static_cast<ALsizei>(info->rate);

  auto offset = 0;
  constexpr auto length = 2014 * 8;
  std::array<uint8_t, length> array{};

  std::vector<uint8_t> data;
  data.reserve(static_cast<size_t>(ov_pcm_total(vf.get(), -1)) * info->channels * 2);

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
  constexpr const auto bigendian = 0;
#else
  constexpr const auto bigendian = 1;
#endif

  do {
    offset = ov_read(vf.get(), reinterpret_cast<char *>(array.data()), length, bigendian, 2, 1, nullptr);
    if (offset < 0) [[unlikely]] {
      throw std::runtime_error(fmt::format("[ov_read] error while reading file: {}, error: {}", filename, ov_strerror(offset)));
    }
    data.insert(data.end(), array.begin(), std::ranges::next(array.begin(), offset));
  } while (offset > 0);

  ALuint buffer;
  alGenBuffers(1, &buffer);
  alBufferData(buffer, format, data.data(), static_cast<ALsizei>(data.size()), frequency);

  alGenSources(1, &_source);
  alSourcei(_source, AL_BUFFER, buffer);
  alDeleteBuffers(1, &buffer);
}

soundfx::~soundfx() {
  alDeleteSources(1, &_source);
}

void soundfx::play(bool loop) const {
  alSourcei(_source, AL_LOOPING, loop);
  alSourcePlay(_source);
}

void soundfx::stop() const {
  alSourceStop(_source);
}
