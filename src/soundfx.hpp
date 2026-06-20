#pragma once

#include "common.hpp"

struct OggOpusFile;

class soundfx final {
public:
  struct stream final {
    ma_data_source_base base{};
    OggOpusFile* file{nullptr};
  };

  explicit soundfx(std::string_view filename);
  ~soundfx();

  soundfx(const soundfx&) = delete;
  soundfx& operator=(const soundfx&) = delete;
  soundfx(soundfx&&) = delete;
  soundfx& operator=(soundfx&&) = delete;

  void play(bool loop);
  void stop() noexcept;

  void update(float delta);

  void set_volume(float gain) noexcept;
  float volume() const noexcept;

  void set_onbegin(sol::protected_function callback);
  void set_onend(sol::protected_function callback);

private:
  std::unique_ptr<PHYSFS_File, PHYSFS_Deleter> _file;
  stream _stream{};
  ma_sound _sound{};

  functor _onbegin;
  functor _onend;
  std::atomic<bool> _ended{false};
};
