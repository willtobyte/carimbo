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
    auto out = context.out();
    out = std::format_to(out, "point({}, {})", p.x(), p.y());
    return out;
  }
};

template<>
struct std::formatter<geometry::rectangle> {
  constexpr auto parse(std::format_parse_context& context) {
    return context.begin();
  }

  auto format(const geometry::rectangle& r, std::format_context& context) const {
    auto out = context.out();
    out = std::format_to(out, "rectangle({}, {}, {}x{})", r.x(), r.y(), r.width(), r.height());
    return out;
  }
};

template<>
struct std::formatter<geometry::size> {
  constexpr auto parse(std::format_parse_context& context) {
    return context.begin();
  }

  auto format(const geometry::size& s, std::format_context& context) const {
    auto out = context.out();
    out = std::format_to(out, "size({}x{})", s.width(), s.height());
    return out;
  }
};

template<>
struct std::formatter<geometry::margin> {
  constexpr auto parse(std::format_parse_context& context) {
    return context.begin();
  }

  auto format(const geometry::margin& m, std::format_context& context) const {
    auto out = context.out();
    out = std::format_to(out, "margin(t:{}, l:{}, b:{}, r:{})", m.top(), m.left(), m.bottom(), m.right());
    return out;
  }
};

template<>
struct std::formatter<graphics::color> {
  constexpr auto parse(std::format_parse_context& context) {
    return context.begin();
  }

  auto format(const graphics::color& c, std::format_context& context) const {
    auto out = context.out();
    out = std::format_to(out, "color(#{:02x}{:02x}{:02x}{:02x})", c.r(), c.g(), c.b(), c.a());
    return out;
  }
};

template<>
struct std::formatter<framework::object> {
  constexpr auto parse(std::format_parse_context& context) {
    return context.begin();
  }

  auto format(const framework::object& o, std::format_context& context) const {
    auto out = context.out();
    out = std::format_to(out, "object(placement:{}, angle:{}, alpha:{}, visible:{}, action:{})", 
      o.placement(), o.angle(), o.alpha(), o.visible(), o.action());
    return out;
  }
};