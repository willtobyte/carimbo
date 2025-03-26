#include "color.hpp"
#include <stdexcept>
#include <string>
#include <algorithm> // For std::transform
#include <cmath>
#include <sstream>
#include <iomanip>   // For std::setw, std::setfill
#include <limits>    // For numeric_limits

// If using fmt, include it here
// #include <fmt/format.h>

using namespace graphics;

// --- Helper Function Implementation ---
constexpr float color::hue_to_rgb(float p, float q, float t) {
    if (t < 0.0f) t += 1.0f;
    if (t > 1.0f) t -= 1.0f;
    if (t < 1.0f / 6.0f) return p + (q - p) * 6.0f * t;
    if (t < 1.0f / 2.0f) return q;
    if (t < 2.0f / 3.0f) return p + (q - p) * (2.0f / 3.0f - t) * 6.0f;
    return p;
}

// --- Constructor Implementations ---
constexpr color::color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) noexcept
    : _r(r), _g(g), _b(b), _a(a) {}

constexpr color::color(const SDL_Color& scolor) noexcept
    : color(scolor.r, scolor.g, scolor.b, scolor.a) {}

// Float constructor
constexpr color::color(float r, float g, float b, float a) noexcept
    : _r(static_cast<uint8_t>(std::clamp(r * 255.0f, 0.0f, 255.0f))),
      _g(static_cast<uint8_t>(std::clamp(g * 255.0f, 0.0f, 255.0f))),
      _b(static_cast<uint8_t>(std::clamp(b * 255.0f, 0.0f, 255.0f))),
      _a(static_cast<uint8_t>(std::clamp(a * 255.0f, 0.0f, 255.0f))) {}

// Hex constructor - Improved
color::color(const std::string& hex_input) : _r(0), _g(0), _b(0), _a(255) {
    if (hex_input.empty() || hex_input[0] != '#') {
        // Use fmt::format if available, otherwise fallback
        // throw std::invalid_argument(fmt::format("Hex code '{}' must start with '#'.", hex_input));
         throw std::invalid_argument("Hex code must start with '#'. Invalid input: " + hex_input);
    }

    std::string hex = hex_input.substr(1); // Skip '#'
    std::transform(hex.begin(), hex.end(), hex.begin(), ::tolower); // Case-insensitive

    try {
        if (hex.length() == 3) { // #RGB format
            _r = static_cast<uint8_t>(std::stoi(std::string(2, hex[0]), nullptr, 16));
            _g = static_cast<uint8_t>(std::stoi(std::string(2, hex[1]), nullptr, 16));
            _b = static_cast<uint8_t>(std::stoi(std::string(2, hex[2]), nullptr, 16));
            _a = 255;
        } else if (hex.length() == 4) { // #RGBA format
             _r = static_cast<uint8_t>(std::stoi(std::string(2, hex[0]), nullptr, 16));
            _g = static_cast<uint8_t>(std::stoi(std::string(2, hex[1]), nullptr, 16));
            _b = static_cast<uint8_t>(std::stoi(std::string(2, hex[2]), nullptr, 16));
            _a = static_cast<uint8_t>(std::stoi(std::string(2, hex[3]), nullptr, 16));
        } else if (hex.length() == 6) { // #RRGGBB format
            _r = static_cast<uint8_t>(std::stoi(hex.substr(0, 2), nullptr, 16));
            _g = static_cast<uint8_t>(std::stoi(hex.substr(2, 2), nullptr, 16));
            _b = static_cast<uint8_t>(std::stoi(hex.substr(4, 2), nullptr, 16));
            _a = 255;
        } else if (hex.length() == 8) { // #RRGGBBAA format
            _r = static_cast<uint8_t>(std::stoi(hex.substr(0, 2), nullptr, 16));
            _g = static_cast<uint8_t>(std::stoi(hex.substr(2, 2), nullptr, 16));
            _b = static_cast<uint8_t>(std::stoi(hex.substr(4, 2), nullptr, 16));
            _a = static_cast<uint8_t>(std::stoi(hex.substr(6, 2), nullptr, 16));
        } else {
            // throw std::invalid_argument(fmt::format("Invalid hex code format: '{}'. Use #RGB, #RGBA, #RRGGBB, or #RRGGBBAA.", hex_input));
             throw std::invalid_argument("Invalid hex code format. Use #RGB, #RGBA, #RRGGBB, or #RRGGBBAA. Invalid input: " + hex_input);
        }
    } catch (const std::invalid_argument& e) {
         // throw std::invalid_argument(fmt::format("Invalid character in hex code '{}': {}", hex_input, e.what()));
         throw std::invalid_argument("Invalid character in hex code '" + hex_input + "': " + e.what());
    } catch (const std::out_of_range& e) {
         // throw std::invalid_argument(fmt::format("Hex value out of range in '{}': {}", hex_input, e.what()));
         throw std::invalid_argument("Hex value out of range in '" + hex_input + "': " + e.what());
    }
}

color::color(uint32_t pixel, const SDL_PixelFormat* format) noexcept {
    SDL_GetRGBA(pixel, format, &_r, &_g, &_b, &_a);
}

// --- Static Factory Implementations ---
color color::from_hsl(float h, float s, float l, float a) noexcept {
    h = std::fmod(h, 360.0f);
    if (h < 0.0f) h += 360.0f;
    s = std::clamp(s, 0.0f, 1.0f);
    l = std::clamp(l, 0.0f, 1.0f);

    float r_f, g_f, b_f;

    if (s == 0.0f) {
        r_f = g_f = b_f = l; // achromatic
    } else {
        float q = l < 0.5f ? l * (1.0f + s) : l + s - l * s;
        float p = 2.0f * l - q;
        float hk = h / 360.0f;
        r_f = hue_to_rgb(p, q, hk + 1.0f / 3.0f);
        g_f = hue_to_rgb(p, q, hk);
        b_f = hue_to_rgb(p, q, hk - 1.0f / 3.0f);
    }

    return color(r_f, g_f, b_f, a);
}

color color::from_hsv(float h, float s, float v, float a) noexcept {
    h = std::fmod(h, 360.0f);
     if (h < 0.0f) h += 360.0f;
    s = std::clamp(s, 0.0f, 1.0f);
    v = std::clamp(v, 0.0f, 1.0f);

    float r_f = v, g_f = v, b_f = v;

    if (s > std::numeric_limits<float>::epsilon()) { // Check saturation against epsilon
         h /= 60.0f; // Sector 0 to 5
         int i = static_cast<int>(std::floor(h));
         float f = h - i; // Factorial part of h
         float p = v * (1.0f - s);
         float q = v * (1.0f - s * f);
         float t = v * (1.0f - s * (1.0f - f));

         switch (i) {
             case 0: r_f = v; g_f = t; b_f = p; break;
             case 1: r_f = q; g_f = v; b_f = p; break;
             case 2: r_f = p; g_f = v; b_f = t; break;
             case 3: r_f = p; g_f = q; b_f = v; break;
             case 4: r_f = t; g_f = p; b_f = v; break;
             default: r_f = v; g_f = p; b_f = q; break; // case 5
         }
     }

    return color(r_f, g_f, b_f, a);
}

constexpr color color::from_uint32(uint32_t argb) noexcept {
    return color(
        static_cast<uint8_t>((argb >> 16) & 0xFF), // R
        static_cast<uint8_t>((argb >> 8) & 0xFF),  // G
        static_cast<uint8_t>(argb & 0xFF),         // B
        static_cast<uint8_t>((argb >> 24) & 0xFF)  // A
    );
}

constexpr color color::from_grayscale(uint8_t gray, uint8_t alpha) noexcept {
    return color(gray, gray, gray, alpha);
}


// --- Color Space Conversion Implementations ---
hsl_color color::to_hsl() const noexcept {
    float r = r_float();
    float g = g_float();
    float b = b_float();

    float max_val = std::max({r, g, b});
    float min_val = std::min({r, g, b});
    float delta = max_val - min_val;

    hsl_color hsl;
    hsl.l = (max_val + min_val) / 2.0f;

    if (delta < std::numeric_limits<float>::epsilon()) {
        hsl.h = 0.0f; // achromatic
        hsl.s = 0.0f;
    } else {
        hsl.s = hsl.l > 0.5f ? delta / (2.0f - max_val - min_val) : delta / (max_val + min_val);
        if (max_val == r) {
            hsl.h = (g - b) / delta + (g < b ? 6.0f : 0.0f);
        } else if (max_val == g) {
            hsl.h = (b - r) / delta + 2.0f;
        } else { // max_val == b
            hsl.h = (r - g) / delta + 4.0f;
        }
        hsl.h *= 60.0f;
         if (hsl.h < 0.0f) hsl.h += 360.0f;
    }
    return hsl;
}

hsv_color color::to_hsv() const noexcept {
    float r = r_float();
    float g = g_float();
    float b = b_float();

    float max_val = std::max({r, g, b});
    float min_val = std::min({r, g, b});
    float delta = max_val - min_val;

    hsv_color hsv;
    hsv.v = max_val; // V is the max component

    if (max_val < std::numeric_limits<float>::epsilon()) {
        hsv.s = 0.0f; // Black
    } else {
         hsv.s = delta / max_val; // Saturation
    }

    if (delta < std::numeric_limits<float>::epsilon()) {
        hsv.h = 0.0f; // achromatic
    } else {
        if (max_val == r) {
            hsv.h = (g - b) / delta + (g < b ? 6.0f : 0.0f);
        } else if (max_val == g) {
            hsv.h = (b - r) / delta + 2.0f;
        } else { // max_val == b
            hsv.h = (r - g) / delta + 4.0f;
        }
        hsv.h *= 60.0f;
         if (hsv.h < 0.0f) hsv.h += 360.0f;
    }
    return hsv;
}

// --- Color Operation Implementations ---
constexpr color color::inverted() const noexcept {
    return color(255 - _r, 255 - _g, 255 - _b, _a);
}

constexpr float color::luminance() const noexcept {
    // Using standard coefficients for relative luminance (Rec. 709)
    return 0.2126f * r_float() + 0.7152f * g_float() + 0.0722f * b_float();
}

color color::grayscale() const noexcept {
    // Convert to grayscale using luminance
    float lum = luminance();
    uint8_t gray_val = static_cast<uint8_t>(std::clamp(lum * 255.0f, 0.0f, 255.0f));
    return color(gray_val, gray_val, gray_val, _a);
}

constexpr color color::with_alpha(uint8_t new_alpha) const noexcept {
    return color(_r, _g, _b, new_alpha);
}

constexpr color color::with_alpha_float(float new_alpha) const noexcept {
    return color(_r, _g, _b, static_cast<uint8_t>(std::clamp(new_alpha * 255.0f, 0.0f, 255.0f)));
}

constexpr color color::adjust_brightness(float factor) const noexcept {
    factor = std::max(0.0f, factor); // Ensure factor is not negative
    return color(
        static_cast<uint8_t>(std::clamp(static_cast<float>(_r) * factor, 0.0f, 255.0f)),
        static_cast<uint8_t>(std::clamp(static_cast<float>(_g) * factor, 0.0f, 255.0f)),
        static_cast<uint8_t>(std::clamp(static_cast<float>(_b) * factor, 0.0f, 255.0f)),
        _a
    );
}

color color::adjust_saturation(float factor) const noexcept {
    hsv_color hsv = to_hsv();
    hsv.s = std::clamp(hsv.s * factor, 0.0f, 1.0f);
    return color::from_hsv(hsv.h, hsv.s, hsv.v, a_float());
}


// --- Blending Implementation ---
constexpr color color::lerp(const color& start, const color& end, float t) noexcept {
    t = std::clamp(t, 0.0f, 1.0f);
    float inv_t = 1.0f - t;
    return color(
        static_cast<uint8_t>(std::round(start._r * inv_t + end._r * t)),
        static_cast<uint8_t>(std::round(start._g * inv_t + end._g * t)),
        static_cast<uint8_t>(std::round(start._b * inv_t + end._b * t)),
        static_cast<uint8_t>(std::round(start._a * inv_t + end._a * t))
    );
}

// --- Operator Implementations ---
constexpr bool color::operator==(const color& other) const noexcept {
    // Use direct comparison for constexpr
    return _r == other._r && _g == other._g && _b == other._b && _a == other._a;
    // Old version using std::tie: return std::tie(_r, _g, _b, _a) == std::tie(other._r, other._g, other._b, other._a);
}

constexpr bool color::operator!=(const color& other) const noexcept {
    return !(*this == other);
}

constexpr bool color::operator<(const color& other) const noexcept {
    // Lexicographical comparison (RGBA order)
     if (_r != other._r) return _r < other._r;
     if (_g != other._g) return _g < other._g;
     if (_b != other._b) return _b < other._b;
     return _a < other._a;
    // Old version using std::tie: return std::tie(_r, _g, _b, _a) < std::tie(other._r, other._g, other._b, other._a);
}


// Helper for clamping addition/subtraction
constexpr uint8_t clamp_add(uint8_t a, uint8_t b) {
    return static_cast<uint8_t>(std::min(255, static_cast<int>(a) + static_cast<int>(b)));
}
constexpr uint8_t clamp_sub(uint8_t a, uint8_t b) {
     return static_cast<uint8_t>(std::max(0, static_cast<int>(a) - static_cast<int>(b)));
}

constexpr color color::operator+(const color& other) const noexcept {
    return color(clamp_add(_r, other._r), clamp_add(_g, other._g), clamp_add(_b, other._b), clamp_add(_a, other._a)); // Add alpha too? Or keep max? Usually max is desired. Let's use max for alpha.
    // return color(clamp_add(_r, other._r), clamp_add(_g, other._g), clamp_add(_b, other._b), std::max(_a, other._a));
}

constexpr color color::operator-(const color& other) const noexcept {
     // Subtracting alpha doesn't usually make sense, keep original alpha
    return color(clamp_sub(_r, other._r), clamp_sub(_g, other._g), clamp_sub(_b, other._b), _a);
}

constexpr color color::operator*(const color& other) const noexcept {
    // Modulate: Component-wise multiplication, normalized
    return color(
        static_cast<uint8_t>((static_cast<int>(_r) * static_cast<int>(other._r)) / 255),
        static_cast<uint8_t>((static_cast<int>(_g) * static_cast<int>(other._g)) / 255),
        static_cast<uint8_t>((static_cast<int>(_b) * static_cast<int>(other._b)) / 255),
        static_cast<uint8_t>((static_cast<int>(_a) * static_cast<int>(other._a)) / 255) // Also modulate alpha? Or keep max? Modulate seems consistent here.
        // std::max(_a, other._a) // Alternative: keep max alpha
    );
}

constexpr color color::operator*(float scalar) const noexcept {
    // Scales RGB, keeps alpha
    scalar = std::max(0.0f, scalar);
     return color(
        static_cast<uint8_t>(std::clamp(static_cast<float>(_r) * scalar, 0.0f, 255.0f)),
        static_cast<uint8_t>(std::clamp(static_cast<float>(_g) * scalar, 0.0f, 255.0f)),
        static_cast<uint8_t>(std::clamp(static_cast<float>(_b) * scalar, 0.0f, 255.0f)),
        _a
    );
}

constexpr color color::operator/(float scalar) const noexcept {
    // Scales RGB, keeps alpha. Avoid division by zero or near-zero.
    if (std::fabs(scalar) < std::numeric_limits<float>::epsilon()) {
        // Return white or black or throw? Let's return white for brightness=inf
        return color(255, 255, 255, _a);
    }
    return (*this) * (1.0f / scalar);
}


color& color::operator+=(const color& other) noexcept { *this = *this + other; return *this; }
color& color::operator-=(const color& other) noexcept { *this = *this - other; return *this; }
color& color::operator*=(const color& other) noexcept { *this = *this * other; return *this; }
color& color::operator*=(float scalar) noexcept { *this = *this * scalar; return *this; }
color& color::operator/=(float scalar) noexcept { *this = *this / scalar; return *this; }

// --- Conversion Implementations ---
constexpr color::operator SDL_Color() const noexcept {
    return SDL_Color{_r, _g, _b, _a};
}

constexpr color::operator uint32_t() const noexcept {
    // ARGB8888 format
    return (static_cast<uint32_t>(_a) << 24) |
           (static_cast<uint32_t>(_r) << 16) |
           (static_cast<uint32_t>(_g) << 8) |
           static_cast<uint32_t>(_b);
}

std::string color::to_hex_string(bool include_alpha) const {
    std::stringstream ss;
    ss << '#';
    ss << std::hex << std::setfill('0');
    ss << std::setw(2) << static_cast<int>(_r);
    ss << std::setw(2) << static_cast<int>(_g);
    ss << std::setw(2) << static_cast<int>(_b);
    if (include_alpha) {
        ss << std::setw(2) << static_cast<int>(_a);
    }
    return ss.str();
    // Alternative using fmtlib:
    // if (include_alpha) {
    //     return fmt::format("#{:02x}{:02x}{:02x}{:02x}", _r, _g, _b, _a);
    // } else {
    //     return fmt::format("#{:02x}{:02x}{:02x}", _r, _g, _b);
    // }
}

std::string color::to_rgba_string() const {
     std::stringstream ss;
     ss << "rgba(" << static_cast<int>(_r) << ", "
        << static_cast<int>(_g) << ", "
        << static_cast<int>(_b) << ", "
        << static_cast<int>(_a) << ")";
     return ss.str();
    // Alternative using fmtlib:
    // return fmt::format("rgba({}, {}, {}, {})", _r, _g, _b, _a);
}

// --- Predefined Color Definitions ---
// Initialize static members (typically done in the .cpp file)
const color color::Transparent = {0, 0, 0, 0};
const color color::Black = {0, 0, 0, 255};
const color color::White = {255, 255, 255, 255};
const color color::Red = {255, 0, 0, 255};
const color color::Green = {0, 255, 0, 255};
const color color::Blue = {0, 0, 255, 255};
const color color::Cyan = {0, 255, 255, 255};
const color color::Magenta = {255, 0, 255, 255};
const color color::Yellow = {255, 255, 0, 255};
const color color::Gray = {128, 128, 128, 255};
const color color::DarkGray = {64, 64, 64, 255};
const color color::LightGray = {192, 192, 192, 255};
const color color::Orange = {255, 165, 0, 255};
const color color::Purple = {128, 0, 128, 255};
const color color::Brown = {165, 42, 42, 255};
const color color::Pink = {255, 192, 203, 255};
const color color::Lime = {0, 255, 0, 255}; // Same as Green in this context
const color color::Teal = {0, 128, 128, 255};
const color color::Navy = {0, 0, 128, 255};
const color color::Olive = {128, 128, 0, 255};
const color color::Maroon = {128, 0, 0, 255};


// --- Non-member function implementations ---
constexpr color graphics::operator*(float scalar, const color& c) noexcept {
    return c * scalar; // Utilize existing member operator
}

// Stream operator implementation (optional)
// std::ostream& graphics::operator<<(std::ostream& os, const color& c) {
//     os << c.to_rgba_string(); // Or use hex string
//     return os;
// }
