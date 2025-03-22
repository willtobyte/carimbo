#pragma once

#include "common.hpp"

// struct SDL_Deleter {
//   void operator()(SDL_Surface *ptr) const noexcept {
//     if (ptr) {
//       SDL_FreeSurface(ptr);
//     }
//   }

//   void operator()(SDL_Texture *ptr) const noexcept {
//     if (ptr) {
//       SDL_DestroyTexture(ptr);
//     }
//   }

//   void operator()(SDL_Renderer *ptr) const noexcept {
//     if (ptr) {
//       SDL_DestroyRenderer(ptr);
//     }
//   }

//   void operator()(SDL_Window *ptr) const noexcept {
//     if (ptr) {
//       SDL_DestroyWindow(ptr);
//     }
//   }

//   void operator()(SDL_RWops *ptr) const noexcept {
//     if (ptr) {
//       SDL_RWclose(ptr);
//     }
//   }

//   void operator()(SDL_GameController *ptr) const noexcept {
//     if (ptr) {
//       SDL_GameControllerClose(ptr);
//     }
//   }
// };

template <typename T>
inline constexpr bool always_false = false;

struct SDL_Deleter {
  template <typename T>
  void operator()(T *ptr) const noexcept {
    if (ptr) {
      if constexpr (std::is_same_v<T, SDL_cond>) {
        SDL_DestroyCond(ptr);
      } else if constexpr (std::is_same_v<T, SDL_Cursor>) {
        SDL_FreeCursor(ptr);
      } else if constexpr (std::is_same_v<T, SDL_GLContext>) {
        SDL_GL_DeleteContext(ptr);
      } else if constexpr (std::is_same_v<T, SDL_GameController>) {
        SDL_GameControllerClose(ptr);
      } else if constexpr (std::is_same_v<T, SDL_Haptic>) {
        SDL_HapticClose(ptr);
      } else if constexpr (std::is_same_v<T, SDL_Joystick>) {
        SDL_JoystickClose(ptr);
      } else if constexpr (std::is_same_v<T, SDL_mutex>) {
        SDL_DestroyMutex(ptr);
      } else if constexpr (std::is_same_v<T, SDL_Palette>) {
        SDL_FreePalette(ptr);
      } else if constexpr (std::is_same_v<T, SDL_PixelFormat>) {
        SDL_FreeFormat(ptr);
      } else if constexpr (std::is_same_v<T, SDL_Renderer>) {
        SDL_DestroyRenderer(ptr);
      } else if constexpr (std::is_same_v<T, SDL_RWops>) {
        SDL_RWclose(ptr);
      } else if constexpr (std::is_same_v<T, SDL_semaphore>) {
        SDL_DestroySemaphore(ptr);
      } else if constexpr (std::is_same_v<T, SDL_Surface>) {
        SDL_FreeSurface(ptr);
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
