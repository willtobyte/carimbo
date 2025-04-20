#pragma once

#include "common.hpp"

template <typename T>
inline constexpr bool always_false = false;

struct SDL_Deleter {
  template <typename T>
  void operator()(T *ptr) const {
    if (ptr) {
      if constexpr (std::is_same_v<T, SDL_Condition>) {
        SDL_DestroyCond(ptr);
      } else if constexpr (std::is_same_v<T, SDL_Cursor>) {
        SDL_FreeCursor(ptr);
      } else if constexpr (std::is_same_v<T, SDL_GLContext>) {
        SDL_GL_DeleteContext(ptr);
      } else if constexpr (std::is_same_v<T, SDL_Gamepad>) {
        SDL_CloseGamepad(ptr);
      } else if constexpr (std::is_same_v<T, SDL_Haptic>) {
        SDL_HapticClose(ptr);
      } else if constexpr (std::is_same_v<T, SDL_Joystick>) {
        SDL_JoystickClose(ptr);
      } else if constexpr (std::is_same_v<T, SDL_Mutex>) {
        SDL_DestroyMutex(ptr);
      } else if constexpr (std::is_same_v<T, SDL_Palette>) {
        SDL_FreePalette(ptr);
      } else if constexpr (std::is_same_v<T, SDL_PixelFormat>) {
        SDL_FreeFormat(ptr);
      } else if constexpr (std::is_same_v<T, SDL_Renderer>) {
        SDL_DestroyRenderer(ptr);
      } else if constexpr (std::is_same_v<T, SDL_IOStream>) {
        SDL_RWclose(ptr);
      } else if constexpr (std::is_same_v<T, SDL_Semaphore>) {
        SDL_DestroySemaphore(ptr);
      } else if constexpr (std::is_same_v<T, SDL_Surface>) {
        SDL_DestroySurface(ptr);
      } else if constexpr (std::is_same_v<T, SDL_Texture>) {
        SDL_DestroyTexture(ptr);
      } else if constexpr (std::is_same_v<T, SDL_Window>) {
        SDL_DestroyWindow(ptr);
      } else {
        static_assert(always_false<T>, "Unsupported SDL type for deletion.");
      }
    }
  }
};
