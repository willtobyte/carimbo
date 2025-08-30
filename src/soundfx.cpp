#include "soundfx.hpp"
#include "io.hpp"

#include <cassert>
#include <cstdint>
#include <cstring>
#include <format>
#include <limits>
#include <memory>
#include <string>
#include <vector>

#include <vorbis/vorbisfile.h>

#if defined(__APPLE__)
  #include <OpenAL/al.h>
  #include <OpenAL/alc.h>
#else
  #include <AL/al.h>
  #include <AL/alc.h>
#endif

// Detecta endian sem depender de SDL
#if !defined(AUDIO_LIL_ENDIAN) && !defined(AUDIO_BIG_ENDIAN)
  #if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
    #define AUDIO_LIL_ENDIAN 1
  #else
    #define AUDIO_BIG_ENDIAN 1
  #endif
#endif

namespace audio {

namespace {

// Stream de memória para libvorbisfile
struct stream {
  const unsigned char* base{nullptr};
  size_t size{0};
  size_t pos{0};
};

size_t cb_read(void* ptr, size_t size, size_t nmemb, void* src) {
  auto* ms = static_cast<stream*>(src);
  size_t want = size * nmemb;
  size_t left = (ms->pos < ms->size) ? (ms->size - ms->pos) : 0;
  if (!left) return 0;
  size_t take = want < left ? want : left;
  std::memcpy(ptr, ms->base + ms->pos, take);
  ms->pos += take;
  return take / size; // fread semantics
}

int cb_seek(void* src, ogg_int64_t offset, int whence) {
  auto* ms = static_cast<stream*>(src);
  size_t target = 0;

  switch (whence) {
    case SEEK_SET: target = offset < 0 ? 0 : static_cast<size_t>(offset); break;
    case SEEK_CUR: {
      long long cur = static_cast<long long>(ms->pos) + offset;
      if (cur < 0) return -1;
      target = static_cast<size_t>(cur);
      break;
    }
    case SEEK_END: {
      long long end = static_cast<long long>(ms->size) + offset;
      if (end < 0) return -1;
      target = static_cast<size_t>(end);
      break;
    }
    default: assert(false); return -1;
  }

  if (target > ms->size) target = ms->size;
  ms->pos = target;
  return 0;
}

long cb_tell(void* src) {
  auto* ms = static_cast<stream*>(src);
  if (ms->pos > static_cast<size_t>(std::numeric_limits<long>::max())) return -1;
  return static_cast<long>(ms->pos);
}

int cb_close(void* /*src*/) { return 0; }

const char* ov_error(int code) {
  switch (code) {
    case OV_FALSE:      return "ov_read() returned 0";
    case OV_EOF:        return "End of file";
    case OV_HOLE:       return "Hole in bitstream";
    case OV_EREAD:      return "Read error";
    case OV_EFAULT:     return "Internal inconsistency";
    case OV_EIMPL:      return "Not implemented";
    case OV_EINVAL:     return "Invalid argument";
    case OV_ENOTVORBIS: return "Not Vorbis";
    case OV_EBADHEADER: return "Bad header";
    case OV_EVERSION:   return "Unsupported version";
    case OV_EBADLINK:   return "Bad link";
    case OV_ENOSEEK:    return "Not seekable";
  }
  return "Unknown";
}

ov_callbacks MEM = { cb_read, cb_seek, cb_close, cb_tell };

} // anon

soundfx::soundfx(const std::string& name) {
  const std::vector<std::uint8_t> buffer = storage::io::read(name);

  stream mem{buffer.data(), buffer.size(), 0};
  std::unique_ptr<OggVorbis_File, decltype(&ov_clear)> vf{new OggVorbis_File, ov_clear};
  int rc = ov_open_callbacks(&mem, vf.get(), nullptr, 0, MEM);
  if (rc < 0) [[unlikely]] {
    throw std::runtime_error(std::format("[ov_open_callbacks(mem)] {} ({}: {})", name, rc, ov_error(rc)));
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

  std::vector<std::uint8_t> pcm16(total);

#if defined(AUDIO_LIL_ENDIAN)
  constexpr int bigendian = 0;
#else
  constexpr int bigendian = 1;
#endif

  size_t offset = 0;
  for (;;) {
    size_t available = pcm16.size() - offset;
    if (!available) break;

    int to_read = (available > static_cast<size_t>(static_cast<unsigned int>(std::numeric_limits<int>::max())))
                  ? std::numeric_limits<int>::max()
                  : static_cast<int>(available);

    long got = ov_read(
      vf.get(),
      reinterpret_cast<char*>(pcm16.data() + offset),
      to_read,
      bigendian,
      2,
      1,
      nullptr
    );

    if (got < 0) [[unlikely]] {
      throw std::runtime_error(std::format("[ov_read] {} | {}", name, ov_error(static_cast<int>(got))));
    } else if (!got) {
      break;
    }

    offset += static_cast<size_t>(got);
  }

  pcm16.resize(offset);

  alGenBuffers(1, &_buffer);
  alBufferData(_buffer, format, pcm16.data(), static_cast<ALsizei>(pcm16.size()), frequency);

  alGenSources(1, &_source);
  alSourcei(_source, AL_BUFFER, static_cast<ALint>(_buffer));
}

soundfx::~soundfx() noexcept {
  if (_source) alDeleteSources(1, &_source);
  if (_buffer) alDeleteBuffers(1, &_buffer);
}

void soundfx::play(bool loop) const noexcept {
  alSourcei(_source, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
  alSourcePlay(_source);
}

void soundfx::stop() const noexcept {
  alSourceStop(_source);
}

}
