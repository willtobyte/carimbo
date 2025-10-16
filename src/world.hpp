#pragma once

#include "common.hpp"

namespace bgi = boost::geometry::index;

using point_t = boost::geometry::model::d2::point_xy<float>;
using box_t = boost::geometry::model::box<point_t>;

namespace framework {
static inline box_t to_box(const geometry::rectangle& r) noexcept {
  return box_t(point_t(r.x(), r.y()), point_t(r.x() + r.width(), r.y() + r.height()));
}

template <class OutIt>
struct resolve_out_iterator final {
  OutIt out;
  const std::unordered_map<uint64_t, std::weak_ptr<object>>* index;
  using iterator_category = std::output_iterator_tag;
  using difference_type = void;
  using value_type = void;
  using pointer = void;
  using reference = void;
  resolve_out_iterator& operator*() noexcept { return *this; }
  resolve_out_iterator& operator++() noexcept { return *this; }
  resolve_out_iterator operator++(int) noexcept { return *this; }
  resolve_out_iterator& operator=(const std::pair<box_t, uint64_t>& p) {
    const auto it = index->find(p.second);
    if (it == index->end()) return *this;
    if (it->second.expired()) return *this;
    *out = it->second;
    ++out;
    return *this;
  }
};

class world final {
  public:
    world() noexcept;
    ~world() noexcept = default;

    void add(const std::shared_ptr<object>& object);
    void remove(const std::shared_ptr<object>& object);

    template <class OutIt>
    void query(float x, float y, OutIt out) {
      auto sink = resolve_out_iterator{out, &_index};
      _spatial.query(bgi::intersects(point_t{x,y}), sink);
    }

    template <class OutIt>
    void query(float x, float y, float w, float h, OutIt out) {
      const box_t area(
        point_t(std::min(x, x+w), std::min(y, y+h)),
        point_t(std::max(x, x+w), std::max(y, y+h))
      );

      auto sink = resolve_out_iterator{out, &_index};
      _spatial.query(bgi::intersects(area), sink);
    }

    // void set_camera(std::shared_ptr<camera> camera) noexcept;

    void update(float delta) noexcept;
    void draw() const noexcept;

  private:
    std::vector<uint64_t> _dirties;

    std::unordered_map<uint64_t, std::weak_ptr<object>> _index;

    std::unordered_map<uint64_t, box_t> _aabbs;

    bgi::rtree<std::pair<box_t, uint64_t>, bgi::rstar<16>> _spatial;

    std::vector<std::pair<box_t, uint64_t>> _hits;

    boost::unordered_set<std::pair<uint64_t,uint64_t>, boost::hash<std::pair<uint64_t,uint64_t>>> _pairs;

    std::shared_ptr<uniquepool<envelope, framework::envelope_pool_name>> _envelopepool = envelopepool::instance();
};
}
