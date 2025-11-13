#include "soundfx.hpp"

using namespace audio;

namespace {
  static size_t cb_read(void* ptr, size_t size, size_t nmemb, void* source) {
    const auto file = reinterpret_cast<PHYSFS_file*> (source);

    PHYSFS_sint64 result
      = PHYSFS_readBytes(file, ptr, static_cast<PHYSFS_uint32>(size) * static_cast<PHYSFS_uint32>(nmemb));
    if (result <= 0) [[unlikely]] {
      return 0;
    }

    return static_cast<size_t> (result) / size;
  }

  static int cb_seek(void* source, ogg_int64_t offset, int whence) {
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

  static int cb_close(void* source) {
    const auto file = reinterpret_cast<PHYSFS_file*> (source);
    PHYSFS_close(file);
    return 0;
  }

  static long cb_tell(void* source) {
    const auto file = reinterpret_cast<PHYSFS_file*> (source);
    return static_cast<long>(PHYSFS_tell(file));
  }
}

soundfx::soundfx(std::string_view filename) {
  const auto ptr = std::unique_ptr<PHYSFS_File, PHYSFS_Deleter>(PHYSFS_openRead(filename.data()));

  if (!ptr) [[unlikely]] {
    throw std::runtime_error(
      std::format("[PHYSFS_openRead] error while opening file: {}, error: {}",
        filename,
        PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode())));
  }

  PHYSFS_setBuffer(ptr.get(), 4 * 1024 * 1024);

  std::unique_ptr<OggVorbis_File, OggVorbis_Deleter> vf{new OggVorbis_File};
  ov_callbacks callbacks = {cb_read, cb_seek, cb_close, cb_tell};
  ov_open_callbacks(ptr.get(), vf.get(), nullptr, 0, callbacks);

  const auto* props = ov_info(vf.get(), -1);
  if (!props) [[unlikely]] {
    throw std::runtime_error(std::format("[ov_info] {}", filename));
  }

  const auto format = (props->channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
  const auto frequency = static_cast<ALsizei>(props->rate);
  const auto estimated = static_cast<size_t>(ov_pcm_total(vf.get(), -1)) * props->channels * 2;

  std::vector<uint8_t> linear16;
  linear16.reserve(estimated);

  std::array<char, 1024 * 1024> buffer{};

  for (;;) {
    auto got = ov_read(
      vf.get(),
      buffer.data(),
      static_cast<int>(buffer.size()),
      0, // little-endian
      2, // 16-bit
      1, // signed
      nullptr
    );

    if (got < 0) [[unlikely]] {
      std::string_view reason;
      switch (got) {
        case OV_HOLE:     reason = "OV_HOLE: Interruption or corruption in the stream"; break;
        case OV_EBADLINK: reason = "OV_EBADLINK: Invalid or corrupt bitstream section"; break;
        case OV_EINVAL:   reason = "OV_EINVAL: Invalid argument or corrupted stream"; break;
        default:          reason = "Unknown error"; break;
      }

      throw std::runtime_error(std::format("[ov_read] {} ({})", filename, reason));
    }

    if (!got) {
      break;
    }

    linear16.insert(linear16.end(), buffer.data(), buffer.data() + got);
  }

  alGenBuffers(1, &_buffer);
  alBufferData(_buffer, format, linear16.data(), static_cast<ALsizei>(linear16.size()), frequency);

  alGenSources(1, &_source);
  alSourcei(_source, AL_BUFFER, static_cast<ALint>(_buffer));
}

soundfx::~soundfx() {
  if (_source == 0) {
    goto freebuffer;
  }

  if (_source) {
    alSourceStop(_source);
    alSourcei(_source, AL_BUFFER, 0);
    alDeleteSources(1, &_source);
    _source = 0;
  }

  freebuffer:
    if (_buffer) {
      alDeleteBuffers(1, &_buffer);
      _buffer = 0;
    }
}

void soundfx::play(bool loop) const {
  _notified.store(false, std::memory_order_relaxed);
  alSourcei(_source, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
  alSourcePlay(_source);

  if (const auto& fn = _onbegin; fn) {
    fn();
  }
}

void soundfx::stop() const {
  alSourceStop(_source);
}

void soundfx::update(float delta) {
  if (_notified.load(std::memory_order_relaxed)) {
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

void soundfx::set_volume(float gain) {
  alSourcef(_source, AL_GAIN, std::clamp(gain, .0f, 1.0f));
}

float soundfx::volume() const {
  float gain;
  alGetSourcef(_source, AL_GAIN, &gain);
  return gain;
}

void soundfx::set_onbegin(sol::protected_function callback) {
  _onbegin = interop::wrap_fn(std::move(callback));
}

void soundfx::set_onend(sol::protected_function callback) {
  _onend = interop::wrap_fn(std::move(callback));
}
