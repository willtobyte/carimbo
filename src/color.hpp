#ifndef GRAPHICS_COLOR_HPP
#define GRAPHICS_COLOR_HPP

#include <cstdint>
#include <string>
#include <stdexcept>
#include <tuple>        // For comparison
#include <cmath>        // For std::round, std::fmod, std::fabs
#include <algorithm>    // For std::clamp, std::min, std::max
#include <sstream>      // For string conversion (can be replaced with <fmt/format.h>)
#include <functional>   // For std::hash
#include <SDL_pixels.h> // Assuming SDL is used

// Forward declaration if fmt is used and available
// namespace fmt { class format_context; }

namespace graphics {

// Helper structs for HSL and HSV color spaces
struct hsl_color {
    float h = 0.0f; // Hue [0.0, 360.0)
    float s = 0.0f; // Saturation [0.0, 1.0]
    float l = 0.0f; // Lightness [0.0, 1.0]
};

struct hsv_color {
    float h = 0.0f; // Hue [0.0, 360.0)
    float s = 0.0f; // Saturation [0.0, 1.0]
    float v = 0.0f; // Value/Brightness [0.0, 1.0]
};


class color {
private:
    uint8_t _r;
    uint8_t _g;
    uint8_t _b;
    uint8_t _a;

    // --- Helper for HSL/HSV conversion ---
    static constexpr float hue_to_rgb(float p, float q, float t);

public:
    // --- Constructors ---
    constexpr color(uint8_t r = 0, uint8_t g = 0, uint8_t b = 0, uint8_t a = 255) noexcept;
    constexpr color(const SDL_Color& scolor) noexcept;
    explicit color(const std::string& hex); // Can throw std::invalid_argument
    explicit color(uint32_t pixel, const SDL_PixelFormat* format) noexcept;
    constexpr color(float r, float g, float b, float a = 1.0f) noexcept; // Float constructor [0.0, 1.0]

    // --- Static Factory Methods ---
    [[nodiscard]] static color from_hsl(float h, float s, float l, float a = 1.0f) noexcept;
    [[nodiscard]] static color from_hsv(float h, float s, float v, float a = 1.0f) noexcept;
    [[nodiscard]] static constexpr color from_uint32(uint32_t argb) noexcept; // Assumes ARGB8888 format
    [[nodiscard]] static constexpr color from_grayscale(uint8_t gray, uint8_t alpha = 255) noexcept;

    // --- Getters (const, noexcept, nodiscard) ---
    [[nodiscard]] constexpr uint8_t r() const noexcept { return _r; }
    [[nodiscard]] constexpr uint8_t g() const noexcept { return _g; }
    [[nodiscard]] constexpr uint8_t b() const noexcept { return _b; }
    [[nodiscard]] constexpr uint8_t a() const noexcept { return _a; }

    // --- Float Getters [0.0, 1.0] (const, noexcept, nodiscard) ---
    [[nodiscard]] constexpr float r_float() const noexcept { return _r / 255.0f; }
    [[nodiscard]] constexpr float g_float() const noexcept { return _g / 255.0f; }
    [[nodiscard]] constexpr float b_float() const noexcept { return _b / 255.0f; }
    [[nodiscard]] constexpr float a_float() const noexcept { return _a / 255.0f; }

    // --- Setters (noexcept) ---
    constexpr void set_r(uint8_t r) noexcept { _r = r; }
    constexpr void set_g(uint8_t g) noexcept { _g = g; }
    constexpr void set_b(uint8_t b) noexcept { _b = b; }
    constexpr void set_a(uint8_t a) noexcept { _a = a; }

    constexpr void set_r_float(float r) noexcept { _r = static_cast<uint8_t>(std::clamp(r * 255.0f, 0.0f, 255.0f)); }
    constexpr void set_g_float(float g) noexcept { _g = static_cast<uint8_t>(std::clamp(g * 255.0f, 0.0f, 255.0f)); }
    constexpr void set_b_float(float b) noexcept { _b = static_cast<uint8_t>(std::clamp(b * 255.0f, 0.0f, 255.0f)); }
    constexpr void set_a_float(float a) noexcept { _a = static_cast<uint8_t>(std::clamp(a * 255.0f, 0.0f, 255.0f)); }

    // --- Color Space Conversions ---
    [[nodiscard]] hsl_color to_hsl() const noexcept;
    [[nodiscard]] hsv_color to_hsv() const noexcept;

    // --- Color Operations ---
    [[nodiscard]] constexpr color inverted() const noexcept;
    [[nodiscard]] color grayscale() const noexcept; // Uses luminance
    [[nodiscard]] constexpr float luminance() const noexcept; // Relative luminance
    [[nodiscard]] constexpr color with_alpha(uint8_t new_alpha) const noexcept;
    [[nodiscard]] constexpr color with_alpha_float(float new_alpha) const noexcept;
    [[nodiscard]] constexpr color adjust_brightness(float factor) const noexcept; // Multiplies RGB by factor
    [[nodiscard]] color adjust_saturation(float factor) const noexcept; // Adjusts saturation via HSV

    // --- Blending ---
    [[nodiscard]] static constexpr color lerp(const color& start, const color& end, float t) noexcept; // Linear interpolation

    // --- Operators ---
    constexpr bool operator==(const color& other) const noexcept;
    constexpr bool operator!=(const color& other) const noexcept;
    constexpr bool operator<(const color& other) const noexcept; // For use in ordered containers

    // Component-wise clamped arithmetic
    [[nodiscard]] constexpr color operator+(const color& other) const noexcept;
    [[nodiscard]] constexpr color operator-(const color& other) const noexcept;
    [[nodiscard]] constexpr color operator*(const color& other) const noexcept; // Modulate (component-wise multiplication / 255)
    [[nodiscard]] constexpr color operator*(float scalar) const noexcept;      // Scale brightness (RGB)
    [[nodiscard]] constexpr color operator/(float scalar) const noexcept;      // Scale brightness (RGB)

    color& operator+=(const color& other) noexcept;
    color& operator-=(const color& other) noexcept;
    color& operator*=(const color& other) noexcept;
    color& operator*=(float scalar) noexcept;
    color& operator/=(float scalar) noexcept;

    // --- Conversions ---
    [[nodiscard]] constexpr operator SDL_Color() const noexcept;
    [[nodiscard]] constexpr operator uint32_t() const noexcept; // To ARGB8888

    [[nodiscard]] std::string to_hex_string(bool include_alpha = true) const;
    [[nodiscard]] std::string to_rgba_string() const; // e.g., "rgba(r, g, b, a)"

    // --- Predefined Colors ---
    // Basic
    static const color Transparent; static const color Black; static const color White;
    static const color Red; static const color Green; static const color Blue;
    static const color Cyan; static const color Magenta; static const color Yellow;
    // Grays
    static const color Gray; static const color DarkGray; static const color LightGray;
    // Common Web Colors (add more as needed)
    static const color Orange; static const color Purple; static const color Brown;
    static const color Pink; static const color Lime; static const color Teal;
    static const color Navy; static const color Olive; static const color Maroon;
};

// --- Non-member functions ---
[[nodiscard]] constexpr color operator*(float scalar, const color& c) noexcept; // Commutative scalar multiplication

// --- Stream operator (optional) ---
// #include <ostream>
// std::ostream& operator<<(std::ostream& os, const color& c);

// --- Hashing support ---
} // namespace graphics

namespace std {
    template <>
    struct hash<graphics::color> {
        [[nodiscard]] constexpr size_t operator()(const graphics::color& c) const noexcept {
            // Simple hash combination - can be improved if needed
            size_t h1 = std::hash<uint8_t>{}(c.r());
            size_t h2 = std::hash<uint8_t>{}(c.g());
            size_t h3 = std::hash<uint8_t>{}(c.b());
            size_t h4 = std::hash<uint8_t>{}(c.a());
            // Combine hashes (boost::hash_combine style)
            size_t seed = 0;
            seed ^= h1 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= h2 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= h3 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= h4 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            return seed;
        }
    };
} // namespace std

// --- fmt formatter specialization (optional, if using fmtlib) ---
// #include <fmt/format.h>
// template <> struct fmt::formatter<graphics::color> {
//     // Basic formatter, outputs hex code by default
//     constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
//         auto it = ctx.begin(), end = ctx.end();
//         if (it != end && *it != '}') throw format_error("invalid format"); // No arguments supported for now
//         return it;
//     }
//
//     template <typename FormatContext>
//     auto format(const graphics::color& c, FormatContext& ctx) const -> decltype(ctx.out()) {
//         return fmt::format_to(ctx.out(), "{}", c.to_hex_string(true));
//     }
// };


#endif // GRAPHICS_COLOR_HPP
