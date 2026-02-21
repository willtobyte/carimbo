#include "soundfx.hpp"

#include "io.hpp"

#include <opusfile.h>

soundfx::soundfx(std::string_view filename) {
  int channels = 0;
  std::vector<float> samples;

  {
    const auto buffer = io::read(filename);

    auto error = 0;
    const std::unique_ptr<OggOpusFile, decltype(&op_free)> opus(
      op_open_memory(buffer.data(), buffer.size(), &error),
      &op_free
    );

    assert((error == 0)
      && std::format("[op_open_memory] failed to decode: {}", filename).c_str());

    channels = op_channel_count(opus.get(), -1);
    const auto nsamples = op_pcm_total(opus.get(), -1);
    const auto total = static_cast<size_t>(nsamples) * static_cast<size_t>(channels);

    samples.resize(total);

    size_t offset = 0;
    while (offset < total) {
      const auto read = op_read_float(
        opus.get(),
        samples.data() + offset,
        static_cast<int>(total - offset),
        nullptr
      );

      if (read == OP_HOLE) {
        continue;
      }

      assert((read >= 0)
        && std::format("[op_read_float] failed to decode: {}", filename).c_str());

      if (read == 0) {
        break;
      }

      offset += static_cast<size_t>(read) * static_cast<size_t>(channels);
    }

    samples.resize(offset);
  }

  auto config = ma_audio_buffer_config_init(
    ma_format_f32,
    static_cast<ma_uint32>(channels),
    samples.size() / static_cast<size_t>(channels),
    samples.data(),
    nullptr
  );
  config.sampleRate = 48000;

  ma_audio_buffer_init(&config, &_buffer);

  ma_sound_init_from_data_source(
    audioengine,
    &_buffer,
    MA_SOUND_FLAG_NO_SPATIALIZATION | MA_SOUND_FLAG_NO_PITCH,
    nullptr,
    &_sound
  );

  ma_sound_set_end_callback(&_sound, [](void* ptr, ma_sound*) {
    static_cast<soundfx*>(ptr)->_ended.store(true, std::memory_order_release);
  }, this);
}

soundfx::~soundfx() {
  ma_sound_uninit(&_sound);
  ma_audio_buffer_uninit(&_buffer);
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
