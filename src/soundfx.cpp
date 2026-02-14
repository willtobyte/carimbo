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

  const auto format = (channels == 1) ? AL_FORMAT_MONO_FLOAT32 : AL_FORMAT_STEREO_FLOAT32;
  constexpr ALsizei frequency = 48000;

  alGenBuffers(1, &_buffer);
  alBufferData(_buffer, format, samples.data(), static_cast<ALsizei>(samples.size() * sizeof(float)), frequency);

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
