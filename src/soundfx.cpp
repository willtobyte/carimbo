#include "soundfx.hpp"

using namespace audio;

namespace {
  static size_t cb_read(void* ptr, size_t size, size_t nmemb, void* source)
  {
    const auto file = reinterpret_cast<PHYSFS_file*> (source);

    PHYSFS_sint64 result
      = PHYSFS_readBytes(file, ptr, static_cast<PHYSFS_uint32>(size) * static_cast<PHYSFS_uint32>(nmemb));
    if (result <= 0) [[unlikely]] {
      return 0;
    }

    return static_cast<size_t> (result) / size;
  }

  static int cb_seek(void* source, ogg_int64_t offset, int whence)
  {
    auto* file = reinterpret_cast<PHYSFS_File*>(source);

    switch (whence) {
      case SEEK_SET: {
        if (offset < 0) [[unlikely]] {
          return -1;
        }

        const auto ok = PHYSFS_seek(file, static_cast<PHYSFS_uint64>(offset));
        return ok ? 0 : -1;
      }

      case SEEK_CUR: {
        const PHYSFS_sint64 position = PHYSFS_tell(file);
        if (position < 0) [[unlikely]] {
          return -1;
        }

        if ((offset > 0 && position > (std::numeric_limits<PHYSFS_sint64>::max)() - offset) ||
            (offset < 0 && position < (std::numeric_limits<PHYSFS_sint64>::min)() - offset)) [[unlikely]] {
          return -1;
        }

        const PHYSFS_sint64 target = position + offset;
        if (target < 0) [[unlikely]] {
          return -1;
        }

        const auto ok = PHYSFS_seek(file, static_cast<PHYSFS_uint64>(target));
        return ok ? 0 : -1;
      }

      case SEEK_END: {
        const PHYSFS_sint64 end = PHYSFS_fileLength(file);
        if (end < 0) [[unlikely]] {
          return -1;
        }

        if ((offset > 0 && end > (std::numeric_limits<PHYSFS_sint64>::max)() - offset) ||
            (offset < 0 && end < (std::numeric_limits<PHYSFS_sint64>::min)() - offset)) [[unlikely]] {
          return -1;
        }

        const PHYSFS_sint64 target = end + offset;
        if (target < 0) [[unlikely]] {
          return -1;
        }

        const auto ok = PHYSFS_seek(file, static_cast<PHYSFS_uint64>(target));
        return ok ? 0 : -1;
      }
    }

    assert(false); [[unlikely]];
    return -1;
  }

  static int cb_close(void* source)
  {
    const auto file = reinterpret_cast<PHYSFS_file*> (source);
    PHYSFS_close(file);
    return 0;
  }

  static long cb_tell(void* source)
  {
    const auto file = reinterpret_cast<PHYSFS_file*> (source);
    return static_cast<long>(PHYSFS_tell(file));
  }
}

soundfx::soundfx(const std::string& name, bool retro) {
  const auto ptr = std::unique_ptr<PHYSFS_File, decltype(&PHYSFS_close)>(PHYSFS_openRead(name.c_str()), PHYSFS_close);

  if (!ptr) [[unlikely]] {
    throw std::runtime_error(
      std::format("[PHYSFS_openRead] error while opening file: {}, error: {}",
        name,
        PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode())));
  }

  PHYSFS_setBuffer(ptr.get(), 4 * 1024 * 1024);

  std::unique_ptr<OggVorbis_File, decltype(&ov_clear)> vf{new OggVorbis_File, ov_clear};
  ov_callbacks callbacks = { cb_read, cb_seek, cb_close, cb_tell };
  ov_open_callbacks(ptr.get(), vf.get(), nullptr, 0, callbacks);

  if (retro) {
    ov_halfrate(vf.get(), 1);
  }

  const auto* info = ov_info(vf.get(), -1);
  if (!info) [[unlikely]] {
    throw std::runtime_error(std::format("[ov_info] {}", name));
  }

  const auto format = (info->channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
  const auto frequency = static_cast<ALsizei>(info->rate);

  const auto pcm_total = ov_pcm_total(vf.get(), -1);
  if (pcm_total < 0) [[unlikely]] {
    throw std::runtime_error(std::format("[ov_pcm_total] {}", name));
  }

  const auto total = static_cast<size_t>(
    static_cast<size_t>(pcm_total) *
    static_cast<size_t>(info->channels) *
    2ULL
  );

  std::vector<std::uint8_t> linear16(total);

  auto offset = 0ULL;
  for (;;) {
    auto available = linear16.size() - offset;
    if (!available) {
      break;
    }

    auto to_read =
        (available > static_cast<size_t>(std::numeric_limits<int>::max()))
            ? std::numeric_limits<int>::max()
            : static_cast<int>(available);

    auto got = ov_read(
      vf.get(),                                          // OggVorbis_File*
      reinterpret_cast<char*>(linear16.data() + offset), // buffer
      to_read,                                           // buffer size
      0,                                                 // big endian flag
      2,                                                 // bytes per sample
      1,                                                 // signed (1) / unsigned (0)
      nullptr                                            // bitstream index
    );

    if (got < 0) [[unlikely]] {
      std::string reason;
      switch (got) {
        case OV_HOLE:     reason = "OV_HOLE: Interruption or corruption in the stream"; break;
        case OV_EBADLINK: reason = "OV_EBADLINK: Invalid or corrupt bitstream section"; break;
        case OV_EINVAL:   reason = "OV_EINVAL: Invalid argument or corrupted stream"; break;
        default:          reason = "Unknown error"; break;
      }

      throw std::runtime_error(std::format("[ov_read] {} ({})", name, reason));
    } else if (!got) {
      break;
    }

    offset += static_cast<size_t>(got);
  }

  linear16.resize(offset);

  alGenBuffers(1, &_buffer);
  alBufferData(_buffer, format, linear16.data(), static_cast<ALsizei>(linear16.size()), frequency);

  alGenSources(1, &_source);
  alSourcei(_source, AL_BUFFER, static_cast<ALint>(_buffer));
}

soundfx::~soundfx() noexcept {
  if (_source == 0) {
    goto free_buffer;
  }

  if (_source) {
    alSourceStop(_source);
    alSourcei(_source, AL_BUFFER, 0);
    alDeleteSources(1, &_source);
    _source = 0;
  }

  free_buffer:
    if (_buffer) {
      alDeleteBuffers(1, &_buffer);
      _buffer = 0;
    }
}

void soundfx::play(bool loop) const noexcept {
  _notified.store(false, std::memory_order_relaxed);
  alSourcei(_source, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
  alSourcePlay(_source);
}

void soundfx::stop() const noexcept {
  alSourceStop(_source);
}

void soundfx::update(float_t delta) noexcept {
  UNUSED(delta);

  if (_notified.load(std::memory_order_relaxed)) [[likely]] {
    return;
  }

  auto state = AL_INITIAL;
  alGetSourcei(_source, AL_SOURCE_STATE, &state);

  if (state != AL_STOPPED) {
    return;
  }

  _notified.store(true, std::memory_order_relaxed);
  if (const auto& fn = _onend; fn) {
    fn();
  }
}

void soundfx::set_onend(std::function<void()>&& callback) noexcept {
  _onend = std::move(callback);
}
