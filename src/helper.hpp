#pragma once

#ifdef _MSC_VER
  #define CONSTEXPR_IF_NOT_MSVC
#else
  #define CONSTEXPR_IF_NOT_MSVC constexpr
#endif

extern "C" const char* stbi_failure_reason(void);

struct ALC_Deleter final {
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

struct SDL_Deleter final {
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

struct STBI_Deleter final {
  template <typename T>
  void operator()(T* ptr) const noexcept {
    if (!ptr) return;

    if constexpr (requires { stbi_image_free(ptr); }) stbi_image_free(ptr);
  }
};

struct PHYSFS_Deleter final {
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

struct string_hash final {
  using is_transparent = void;

  size_t operator()(std::string_view sv) const noexcept {
    return std::hash<std::string_view>{}(sv);
  }

  size_t operator()(std::string const& s) const noexcept {
    return std::hash<std::string_view>{}(s);
  }
};

struct string_equal final {
  using is_transparent = void;

  bool operator()(std::string_view a, std::string_view b) const noexcept {
    return a == b;
  }

  bool operator()(std::string const& a, std::string const& b) const noexcept {
    return a == b;
  }

  bool operator()(std::string const& a, std::string_view b) const noexcept {
    return std::string_view(a) == b;
  }

  bool operator()(std::string_view a, std::string const& b) const noexcept {
    return a == std::string_view(b);
  }
};

template<typename T, typename D>
[[nodiscard]] constexpr auto unwrap(
  std::unique_ptr<T, D>&& ptr,
  std::string_view message,
  std::source_location location = std::source_location::current())
    -> std::unique_ptr<T, D> {
    if (!ptr) [[unlikely]] {
        throw std::runtime_error(
            std::format("{}:{} - {}",
              location.file_name(),
              location.line(),
              message)
        );
    }

    return std::move(ptr);
}

template<typename T>
[[nodiscard]] inline auto unwrap(
  std::unique_ptr<T, SDL_Deleter>&& ptr,
  std::string_view message,
  std::source_location location = std::source_location::current())
    -> std::unique_ptr<T, SDL_Deleter> {
    if (!ptr) [[unlikely]] {
        throw std::runtime_error(
            std::format("{}:{} - {} | SDL Error: {}",
              location.file_name(),
              location.line(),
              message,
              SDL_GetError())
        );
    }

    return std::move(ptr);
}

template<typename T>
[[nodiscard]] inline auto unwrap(
  std::unique_ptr<T, PHYSFS_Deleter>&& ptr,
  std::string_view message,
  std::source_location location = std::source_location::current())
    -> std::unique_ptr<T, PHYSFS_Deleter> {
    if (!ptr) [[unlikely]] {
        throw std::runtime_error(
            std::format("{}:{} - {} | PHYSFS Error: {}",
              location.file_name(),
              location.line(),
              message,
              PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()))
        );
    }

    return std::move(ptr);
}

template<typename T>
[[nodiscard]] inline auto unwrap(
  std::unique_ptr<T, STBI_Deleter>&& ptr,
  std::string_view message,
  std::source_location location = std::source_location::current())
    -> std::unique_ptr<T, STBI_Deleter> {
    if (!ptr) [[unlikely]] {
        throw std::runtime_error(
            std::format("{}:{} - {} | STBI Error: {}",
              location.file_name(),
              location.line(),
              message,
              stbi_failure_reason())
        );
    }

    return std::move(ptr);
}

template<typename T>
[[nodiscard]] inline auto unwrap(
  std::unique_ptr<T, ALC_Deleter>&& ptr,
  std::string_view message,
  ALCdevice* device = nullptr,
  std::source_location location = std::source_location::current())
    -> std::unique_ptr<T, ALC_Deleter> {
    if (!ptr) [[unlikely]] {
        const auto error = alcGetError(device);
        const char* error_str = "Unknown error";

        switch (error) {
            case ALC_NO_ERROR: error_str = "No error"; break;
            case ALC_INVALID_DEVICE: error_str = "Invalid device"; break;
            case ALC_INVALID_CONTEXT: error_str = "Invalid context"; break;
            case ALC_INVALID_ENUM: error_str = "Invalid enum"; break;
            case ALC_INVALID_VALUE: error_str = "Invalid value"; break;
            case ALC_OUT_OF_MEMORY: error_str = "Out of memory"; break;
        }

        throw std::runtime_error(
            std::format("{}:{} - {} | OpenAL Error: {} ({:#x})",
              location.file_name(),
              location.line(),
              message,
              error_str,
              error)
        );
    }

    return std::move(ptr);
}

struct functor final {
  sol::protected_function fn;

  functor() noexcept = default;
  functor(sol::protected_function f) noexcept : fn(std::move(f)) {}
  functor(std::nullptr_t) noexcept : fn() {}

  functor& operator=(sol::protected_function f) noexcept {
    fn = std::move(f);
    return *this;
  }

  functor& operator=(std::nullptr_t) noexcept {
    fn = sol::protected_function();
    return *this;
  }

  [[nodiscard]] explicit operator bool() const noexcept {
    return fn.valid() && fn.lua_state() != nullptr;
  }

  template<typename... Args>
  void operator()(Args&&... args) const {
    if (!fn.valid() || !fn.lua_state()) [[unlikely]] return;
    auto result = fn(std::forward<Args>(args)...);
    if (!result.valid()) [[unlikely]] {
      sol::error err = result;
      throw std::runtime_error(err.what());
    }
  }

  template<typename R, typename... Args>
  [[nodiscard]] R call(Args&&... args) const {
    if (!fn.valid() || !fn.lua_state()) [[unlikely]] return R{};
    auto result = fn(std::forward<Args>(args)...);
    if (!result.valid()) [[unlikely]] {
      sol::error err = result;
      throw std::runtime_error(err.what());
    }

    return result.template get<R>();
  }
};

template<typename T>
  requires requires(const T& t) { { t.valid() } -> std::convertible_to<bool>; }
inline void verify(const T& result) {
  if (!result.valid()) [[unlikely]] {
    sol::error err = result;
    throw std::runtime_error(err.what());
  }
}