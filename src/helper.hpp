#pragma once

struct ALC_Deleter {
  template <typename T>
  void operator()(T* ptr) const noexcept {
    if (!ptr) return;

    if constexpr (requires { alcCloseDevice(ptr); }) alcCloseDevice(ptr);
    else if constexpr (requires { alcDestroyContext(ptr); }) {
      alcMakeContextCurrent(nullptr);
      alcDestroyContext(ptr);
    }
  }
};

struct SDL_Deleter {
  template <typename T>
  void operator()(T* ptr) const {
    if (!ptr) return;

    if constexpr (requires { SDL_DestroyCond(ptr); }) SDL_DestroyCond(ptr);
    else if constexpr (requires { SDL_FreeCursor(ptr); }) SDL_FreeCursor(ptr);
    else if constexpr (requires { SDL_GL_DeleteContext(ptr); }) SDL_GL_DeleteContext(ptr);
    else if constexpr (requires { SDL_CloseGamepad(ptr); }) SDL_CloseGamepad(ptr);
    else if constexpr (requires { SDL_HapticClose(ptr); }) SDL_HapticClose(ptr);
    else if constexpr (requires { SDL_JoystickClose(ptr); }) SDL_JoystickClose(ptr);
    else if constexpr (requires { SDL_DestroyMutex(ptr); }) SDL_DestroyMutex(ptr);
    else if constexpr (requires { SDL_FreePalette(ptr); }) SDL_FreePalette(ptr);
    else if constexpr (requires { SDL_FreeFormat(ptr); }) SDL_FreeFormat(ptr);
    else if constexpr (requires { SDL_DestroyRenderer(ptr); }) SDL_DestroyRenderer(ptr);
    else if constexpr (requires { SDL_RWclose(ptr); }) SDL_RWclose(ptr);
    else if constexpr (requires { SDL_DestroySemaphore(ptr); }) SDL_DestroySemaphore(ptr);
    else if constexpr (requires { SDL_DestroySurface(ptr); }) SDL_DestroySurface(ptr);
    else if constexpr (requires { SDL_DestroyTexture(ptr); }) SDL_DestroyTexture(ptr);
    else if constexpr (requires { SDL_DestroyWindow(ptr); }) SDL_DestroyWindow(ptr);
    else if constexpr (requires { SDL_free(ptr); }) SDL_free(ptr);
  }
};

struct OggVorbis_Deleter {
  template <typename T>
  void operator()(T* ptr) const noexcept {
    if (!ptr) return;

    if constexpr (requires { ov_clear(ptr); }) ov_clear(ptr);
  }
};

struct SPNG_Deleter {
  template <typename T>
  void operator()(T* ptr) const noexcept {
    if (!ptr) return;

    if constexpr (requires { spng_ctx_free(ptr); }) spng_ctx_free(ptr);
  }
};

struct PHYSFS_Deleter {
  template <typename T>
  void operator()(T* ptr) const noexcept {
    if (!ptr) return;

    if constexpr (std::is_same_v<T, PHYSFS_File>) {
      PHYSFS_close(ptr);
    } else if constexpr (std::is_same_v<T, char*>) {
      PHYSFS_freeList(ptr);
    }
  }
};

struct string_hash {
  using is_transparent = void;
  using hash_type = std::hash<std::string_view>;
  
  size_t operator()(std::string_view str) const noexcept {
    return hash_type{}(str);
  }
  
  size_t operator()(const std::string& str) const noexcept {
    return hash_type{}(str);
  }
  
  size_t operator()(const char* str) const noexcept {
    return hash_type{}(str);
  }
};

template <typename... Ts>
constexpr void UNUSED(const Ts&...) {}

inline uint64_t moment() { return SDL_GetTicks(); }
