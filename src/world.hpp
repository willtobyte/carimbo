#pragma once

#include "common.hpp"

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

    std::vector<std::weak_ptr<object>> query(float x, float y);
    std::vector<std::weak_ptr<object>> query(float x, float y, float w, float h);
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
