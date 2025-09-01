#pragma once

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
  }
};

template <typename... Ts>
constexpr void UNUSED(const Ts&...) {}

inline uint64_t moment() noexcept { return SDL_GetTicks(); }
