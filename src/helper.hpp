#pragma once

struct ALC_Deleter {
  template <typename T>
  void operator()(T* ptr) const noexcept {
    if (!ptr) return;

    if constexpr (requires { alcCloseDevice(ptr); }) alcCloseDevice(ptr);
    if constexpr (requires { alcDestroyContext(ptr); }) {
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
    if constexpr (requires { SDL_FreeCursor(ptr); }) SDL_FreeCursor(ptr);
    if constexpr (requires { SDL_GL_DeleteContext(ptr); }) SDL_GL_DeleteContext(ptr);
    if constexpr (requires { SDL_CloseGamepad(ptr); }) SDL_CloseGamepad(ptr);
    if constexpr (requires { SDL_HapticClose(ptr); }) SDL_HapticClose(ptr);
    if constexpr (requires { SDL_JoystickClose(ptr); }) SDL_JoystickClose(ptr);
    if constexpr (requires { SDL_DestroyMutex(ptr); }) SDL_DestroyMutex(ptr);
    if constexpr (requires { SDL_FreePalette(ptr); }) SDL_FreePalette(ptr);
    if constexpr (requires { SDL_FreeFormat(ptr); }) SDL_FreeFormat(ptr);
    if constexpr (requires { SDL_DestroyRenderer(ptr); }) SDL_DestroyRenderer(ptr);
    if constexpr (requires { SDL_RWclose(ptr); }) SDL_RWclose(ptr);
    if constexpr (requires { SDL_DestroySemaphore(ptr); }) SDL_DestroySemaphore(ptr);
    if constexpr (requires { SDL_DestroySurface(ptr); }) SDL_DestroySurface(ptr);
    if constexpr (requires { SDL_DestroyTexture(ptr); }) SDL_DestroyTexture(ptr);
    if constexpr (requires { SDL_DestroyWindow(ptr); }) SDL_DestroyWindow(ptr);
    if constexpr (requires { SDL_free(ptr); }) SDL_free(ptr);
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

    if constexpr (requires { PHYSFS_close(ptr); }) PHYSFS_close(ptr);
    if constexpr (requires { PHYSFS_freeList(ptr); }) PHYSFS_freeList(ptr);
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
