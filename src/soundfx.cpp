#include "soundfx.hpp"

#include "io.hpp"

#include <stb_vorbis.c>

soundfx::soundfx(std::string_view filename) noexcept {
  const auto buffer = io::read(filename);

  int channels;
  int sample_rate;
  short* output;
  const auto samples = stb_vorbis_decode_memory(
    buffer.data(),
    static_cast<int>(buffer.size()),
    &channels,
    &sample_rate,
    &output
  );

  assert(samples >= 0 && output &&
    std::format("[stb_vorbis_decode_memory] failed to decode: {}", filename).c_str());

  const std::unique_ptr<short, decltype(&free)> decoded(output, &free);

  const auto format = (channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
  const auto frequency = static_cast<ALsizei>(sample_rate);
  const auto size = static_cast<size_t>(samples) * static_cast<size_t>(channels) * sizeof(short);

  alGenBuffers(1, &_buffer);
  alBufferData(_buffer, format, decoded.get(), static_cast<ALsizei>(size), frequency);

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

void soundfx::play(bool loop) const noexcept {
  _notified.store(false, std::memory_order_relaxed);
  alSourcei(_source, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
  alSourcePlay(_source);
  if (const auto fn = _onbegin; fn) {
    fn();
  }
}

void soundfx::stop() const noexcept {
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
  if (const auto fn = _onend; fn) {
    fn();
  }
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