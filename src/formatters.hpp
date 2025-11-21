#pragma once

#include "point.hpp"
#include "rectangle.hpp"
#include "size.hpp"
#include "margin.hpp"
#include "color.hpp"
#include "object.hpp"

template<>
struct std::formatter<geometry::point> {
  constexpr auto parse(std::format_parse_context& context) {
    return context.begin();
  }

  auto format(const geometry::point& p, std::format_context& context) const {
    return std::format_to(context.out(), "point({}, {})", p.x(), p.y());
  }
};

template<>
struct std::formatter<geometry::rectangle> : std::formatter<std::string> {
  auto format(const geometry::rectangle& r, std::format_context& context) const {
    return std::formatter<std::string>::format(
      std::format("rectangle({}, {}, {}x{})", r.x(), r.y(), r.width(), r.height()),
      context
    );
  }
};

template<>
struct std::formatter<geometry::size> : std::formatter<std::string> {
  auto format(const geometry::size& s, std::format_context& context) const {
    return std::formatter<std::string>::format(
      std::format("size({}x{})", s.width(), s.height()),
      context
    );
  }
};

template<>
struct std::formatter<geometry::margin> : std::formatter<std::string> {
  auto format(const geometry::margin& m, std::format_context& context) const {
    return std::formatter<std::string>::format(
      std::format("margin(t:{}, l:{}, b:{}, r:{})", m.top(), m.left(), m.bottom(), m.right()),
      context
    );
  }
};

template<>
struct std::formatter<graphics::color> : std::formatter<std::string> {
  auto format(const graphics::color& c, std::format_context& context) const {
    return std::formatter<std::string>::format(
      std::format("color(#{:02x}{:02x}{:02x}{:02x})", c.r(), c.g(), c.b(), c.a()),
      context
    );
  }
};

template<>
struct std::formatter<framework::object> : std::formatter<std::string> {
  auto format(const framework::object& o, std::format_context& context) const {
    return std::formatter<std::string>::format(
      std::format("object(placement:{}, angle:{}, alpha:{}, visible:{}, action:{})",
        o.placement(), o.angle(), o.alpha(), o.visible(), o.action()),
      context
    );
  }
};
