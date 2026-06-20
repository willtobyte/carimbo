#include "soundfx.hpp"

#include "helper.hpp"

#include <opusfile.h>

namespace {
  ma_result read(ma_data_source* source, void* output, ma_uint64 frames, ma_uint64* count) {
    auto* self = reinterpret_cast<soundfx::stream*>(source);
    const int channels = op_channel_count(self->file, -1);

    ma_uint64 total = 0;
    while (total < frames) {
      const auto remaining = frames - total;
      const auto request = static_cast<int>(std::min<ma_uint64>(
        remaining * static_cast<ma_uint64>(channels),
        static_cast<ma_uint64>(std::numeric_limits<int>::max())
      ));

      auto* destination = static_cast<float*>(output) + total * static_cast<ma_uint64>(channels);
      const int decoded = op_read_float(self->file, destination, request, nullptr);
      if (decoded == OP_HOLE) [[unlikely]] {
        continue;
      }

      if (decoded < 0) [[unlikely]] {
        if (count) [[likely]] {
          *count = total;
        }

        return MA_ERROR;
      }

      if (decoded == 0) [[unlikely]] {
        break;
      }

      total += static_cast<ma_uint64>(decoded);
    }

    if (count) [[likely]] {
      *count = total;
    }

    if (total == 0) [[unlikely]] {
      return MA_AT_END;
    }

    if (total < frames) [[unlikely]] {
      return MA_AT_END;
    }

    return MA_SUCCESS;
  }

  ma_result seek(ma_data_source* source, ma_uint64 index) {
    auto* self = reinterpret_cast<soundfx::stream*>(source);
    const int result = op_pcm_seek(self->file, static_cast<ogg_int64_t>(index));
    if (result == 0) [[likely]] {
      return MA_SUCCESS;
    }

    if (result == OP_ENOSEEK) [[unlikely]] {
      return MA_INVALID_OPERATION;
    }

    return MA_ERROR;
  }

  ma_result format(ma_data_source* source, ma_format* format, ma_uint32* channels, ma_uint32* rate, ma_channel* map, size_t capacity) {
    auto* self = reinterpret_cast<soundfx::stream*>(source);
    const auto count = static_cast<ma_uint32>(op_channel_count(self->file, -1));
    if (format) {
      *format = ma_format_f32;
    }

    if (channels) {
      *channels = count;
    }

    if (rate) {
      *rate = 48000;
    }

    if (map) {
      ma_channel_map_init_standard(ma_standard_channel_map_vorbis, map, capacity, count);
    }

    return MA_SUCCESS;
  }

  ma_result cursor(ma_data_source* source, ma_uint64* cursor) {
    auto* self = reinterpret_cast<soundfx::stream*>(source);
    const auto offset = op_pcm_tell(self->file);
    if (offset < 0) [[unlikely]] {
      return MA_ERROR;
    }

    *cursor = static_cast<ma_uint64>(offset);
    return MA_SUCCESS;
  }

  ma_result length(ma_data_source* source, ma_uint64* length) {
    auto* self = reinterpret_cast<soundfx::stream*>(source);
    const auto total = op_pcm_total(self->file, -1);
    if (total < 0) [[unlikely]] {
      return MA_ERROR;
    }

    *length = static_cast<ma_uint64>(total);
    return MA_SUCCESS;
  }

  constexpr ma_data_source_vtable vtable = {
    read,
    seek,
    format,
    cursor,
    length,
    nullptr,
    0,
  };

  int physfs_read(void* source, unsigned char* output, int bytes) {
    auto* file = static_cast<PHYSFS_File*>(source);
    return static_cast<int>(PHYSFS_readBytes(file, output, static_cast<PHYSFS_uint64>(bytes)));
  }

  int physfs_seek(void* source, opus_int64 offset, int whence) {
    auto* file = static_cast<PHYSFS_File*>(source);

    PHYSFS_sint64 base = 0;
    switch (whence) {
      case SEEK_SET:
        base = 0;
        break;
      case SEEK_CUR:
        base = PHYSFS_tell(file);
        break;
      case SEEK_END:
        base = PHYSFS_fileLength(file);
        break;
      default:
        return -1;
    }

    if (base < 0) [[unlikely]] {
      return -1;
    }

    const auto target = base + offset;
    if (target < 0) [[unlikely]] {
      return -1;
    }

    return PHYSFS_seek(file, static_cast<PHYSFS_uint64>(target)) != 0 ? 0 : -1;
  }

  opus_int64 physfs_tell(void* source) {
    return PHYSFS_tell(static_cast<PHYSFS_File*>(source));
  }

  constexpr OpusFileCallbacks opus_callbacks = {
    physfs_read,
    physfs_seek,
    physfs_tell,
    nullptr,
  };
}

soundfx::soundfx(std::string_view filename) {
  _file = unwrap(
    std::unique_ptr<PHYSFS_File, PHYSFS_Deleter>(PHYSFS_openRead(filename.data())),
    std::format("[PHYSFS_openRead] error while opening file: {}", filename)
  );

  auto error = 0;
  _stream.file = op_open_callbacks(_file.get(), &opus_callbacks, nullptr, 0, &error);

  assert((error == 0)
    && std::format("[op_open_callbacks] failed to decode: {}", filename).c_str());

  auto config = ma_data_source_config_init();
  config.vtable = &vtable;
  ma_data_source_init(&config, &_stream.base);

  ma_sound_init_from_data_source(
    audioengine,
    &_stream.base,
    MA_SOUND_FLAG_NO_SPATIALIZATION | MA_SOUND_FLAG_NO_PITCH,
    nullptr,
    &_sound
  );

  ma_sound_set_end_callback(&_sound, [](void* ptr, ma_sound*) {
    static_cast<soundfx*>(ptr)->_ended.store(true, std::memory_order_release);
  }, this);
}

soundfx::~soundfx() {
  ma_sound_set_end_callback(&_sound, nullptr, nullptr);
  ma_sound_stop(&_sound);
  ma_sound_uninit(&_sound);
  ma_data_source_uninit(&_stream.base);
  op_free(_stream.file);
}

void soundfx::play(bool loop) {
  _ended.store(false, std::memory_order_relaxed);
  ma_sound_seek_to_pcm_frame(&_sound, 0);
  ma_sound_set_looping(&_sound, loop ? MA_TRUE : MA_FALSE);
  ma_sound_start(&_sound);
  _onbegin();
}

void soundfx::stop() noexcept {
  ma_sound_stop(&_sound);
}

void soundfx::update(float delta) {
  if (_ended.exchange(false, std::memory_order_acquire)) {
    _onend();
  }
}

void soundfx::set_volume(float gain) noexcept {
  ma_sound_set_volume(&_sound, std::clamp(gain, .0f, 1.0f));
}

float soundfx::volume() const noexcept {
  return ma_sound_get_volume(&_sound);
}

void soundfx::set_onbegin(sol::protected_function callback) {
  _onbegin = std::move(callback);
}

void soundfx::set_onend(sol::protected_function callback) {
  _onend = std::move(callback);
}
