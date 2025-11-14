#include "soundfx.hpp"

using namespace audio;

namespace {
  constexpr auto KILOBYTE = 1024uz;
  constexpr auto READ_BUFFER_SIZE = 1 * KILOBYTE * KILOBYTE;
  constexpr auto PHYSFS_BUFFER_SIZE = 4 * READ_BUFFER_SIZE;

  static std::array<char, READ_BUFFER_SIZE> buffer;

  template<typename To, typename From>
  auto safe_cast(From value) {
    return static_cast<To>(value);
  }

  static size_t physfs_read(void* ptr, size_t size, size_t nmemb, void* source) {
    const auto result = PHYSFS_readBytes(
      static_cast<PHYSFS_file*>(source),
      ptr,
      safe_cast<PHYSFS_uint32>(size * nmemb)
    );

    return (result > 0) ? safe_cast<size_t>(result) / size : 0;
  }

  static int seek_file(PHYSFS_File* ptr, PHYSFS_sint64 target) {
    return (target < 0 || !PHYSFS_seek(ptr, safe_cast<PHYSFS_uint64>(target))) ? -1 : 0;
  }

  static bool would_overflow(PHYSFS_sint64 base, ogg_int64_t offset) {
    const auto positive = offset > 0;
    const auto limit = positive
      ? (std::numeric_limits<PHYSFS_sint64>::max)()
      : (std::numeric_limits<PHYSFS_sint64>::min)();
    return positive ? (base > limit - offset) : (base < limit - offset);
  }

  static PHYSFS_sint64 compute_seek_target(PHYSFS_File* ptr, ogg_int64_t offset, int whence) {
    switch (whence) {
      case SEEK_SET:
        return (offset < 0) ? -1 : offset;

      case SEEK_CUR: {
        const auto pos = PHYSFS_tell(ptr);
        if (pos < 0 || would_overflow(pos, offset)) [[unlikely]] return -1;
        return pos + offset;
      }

      case SEEK_END: {
        const auto end = PHYSFS_fileLength(ptr);
        if (end < 0 || would_overflow(end, offset)) [[unlikely]] return -1;
        return end + offset;
      }

      default:
        return -1;
    }
  }

  static int physfs_seek(void* source, ogg_int64_t offset, int whence) {
    auto* file = static_cast<PHYSFS_File*>(source);
    return seek_file(file, compute_seek_target(file, offset, whence));
  }

  static int physfs_close(void* source) {
    PHYSFS_close(static_cast<PHYSFS_file*>(source));
    return 0;
  }

  static long physfs_tell(void* source) {
    return safe_cast<long>(PHYSFS_tell(static_cast<PHYSFS_file*>(source)));
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

  PHYSFS_setBuffer(ptr.get(), PHYSFS_BUFFER_SIZE);

  std::unique_ptr<OggVorbis_File, OggVorbis_Deleter> vf{new OggVorbis_File};
  ov_callbacks callbacks = {physfs_read, physfs_seek, physfs_close, physfs_tell};
  ov_open_callbacks(ptr.get(), vf.get(), nullptr, 0, callbacks);

  const auto* props = ov_info(vf.get(), -1);
  const auto format = (props->channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
  const auto frequency = static_cast<ALsizei>(props->rate);
  const auto estimated = static_cast<size_t>(ov_pcm_total(vf.get(), -1)) * props->channels * 2;

  std::vector<uint8_t> linear16;
  linear16.reserve(estimated);

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
        case OV_HOLE:     reason = "OV_HOLE"; break;
        case OV_EBADLINK: reason = "OV_EBADLINK"; break;
        case OV_EINVAL:   reason = "OV_EINVAL"; break;
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
  if (_source != 0) {
    alSourceStop(_source);
    alSourcei(_source, AL_BUFFER, 0);
    alDeleteSources(1, &_source);
    _source = 0;
  }

  if (_buffer != 0) {
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
