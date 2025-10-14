#pragma once

#include "common.hpp"

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/index/rtree.hpp>

namespace bgi = boost::geometry::index;

using point_t = boost::geometry::model::d2::point_xy<float>;
using box_t = boost::geometry::model::box<point_t>;

namespace framework {
class world final {
  public:
    world() noexcept;
    ~world() noexcept = default;

    void add(const std::shared_ptr<object>& object);
    void remove(const std::shared_ptr<object>& object);

    // void set_camera(std::shared_ptr<camera> camera) noexcept;

    void update(float delta) noexcept;
    void draw() const noexcept;

  private:
    std::unordered_map<uint64_t, std::weak_ptr<object>> _index;

    std::unordered_map<uint64_t, box_t> _aabbs;

    bgi::rtree<std::pair<box_t, uint64_t>, bgi::quadratic<16>> _spatial;

    std::shared_ptr<uniquepool<envelope, framework::envelope_pool_name>> _envelopepool = envelopepool::instance();
};
}
