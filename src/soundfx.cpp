#include "soundfx.hpp"

#include "io.hpp"

#include <stb_vorbis.c>

soundfx::soundfx(std::string_view filename) {
  const auto buffer = io::read(filename);

  auto error = 0;
  const std::unique_ptr<stb_vorbis, decltype(&stb_vorbis_close)> vorbis(
    stb_vorbis_open_memory(
      buffer.data(),
      static_cast<int>(buffer.size()),
      &error,
      nullptr
    ),
    &stb_vorbis_close
  );

  assert((error == VORBIS__no_error)
    && std::format("[stb_vorbis_open_memory] failed to decode: {}", filename).c_str());

  const auto info = stb_vorbis_get_info(vorbis.get());
  const auto total_samples = stb_vorbis_stream_length_in_samples(vorbis.get());
  const auto channels = info.channels;
  const auto sample_rate = info.sample_rate;
  const auto total_floats = static_cast<size_t>(total_samples) * static_cast<size_t>(channels);

  auto samples = std::vector<float>(total_floats);

  const auto decoded = stb_vorbis_get_samples_float_interleaved(
    vorbis.get(),
    channels,
    samples.data(),
    static_cast<int>(total_floats)
  );

  assert((decoded > 0)
    && std::format("[stb_vorbis_get_samples_float_interleaved] failed to decode: {}", filename).c_str());

  const auto format = (channels == 1) ? AL_FORMAT_MONO_FLOAT32 : AL_FORMAT_STEREO_FLOAT32;
  const auto frequency = static_cast<ALsizei>(sample_rate);
  const auto size = static_cast<size_t>(decoded) * static_cast<size_t>(channels) * sizeof(float);

  alGenBuffers(1, &_buffer);
  alBufferData(_buffer, format, samples.data(), static_cast<ALsizei>(size), frequency);

  alGenSources(1, &_source);
  alSourcei(_source, AL_BUFFER, static_cast<ALint>(_buffer));
}

soundfx::~soundfx() {
  if (_source) {
    alSourceStop(_source);
    alSourcei(_source, AL_BUFFER, 0);
    alDeleteSources(1, &_source);
  }

  if (_buffer) {
    alDeleteBuffers(1, &_buffer);
  }
}

void soundfx::play(bool loop) const {
  _notified = false;
  alSourcei(_source, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
  alSourcePlay(_source);
  _onbegin();
}

void soundfx::stop() const noexcept {
  alSourceStop(_source);
}

void soundfx::update(float delta) {
  if (_notified) {
    return;
  }

  auto state = AL_INITIAL;
  alGetSourcei(_source, AL_SOURCE_STATE, &state);

  if (state != AL_STOPPED) {
    return;
  }

  _notified = true;
  _onend();
}

void soundfx::set_volume(float gain) noexcept {
  alSourcef(_source, AL_GAIN, std::clamp(gain, .0f, 1.0f));
}

float soundfx::volume() const noexcept {
  float gain;
  alGetSourcef(_source, AL_GAIN, &gain);
  return gain;
}

void soundfx::set_onbegin(sol::protected_function callback) {
  _onbegin = std::move(callback);
}

void soundfx::set_onend(sol::protected_function callback) {
  _onend = std::move(callback);
}
