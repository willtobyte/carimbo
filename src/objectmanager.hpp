#pragma once

#include "common.hpp"

#include "event.hpp"
#include "eventreceiver.hpp"
#include "scenemanager.hpp"
#include "resourcemanager.hpp"
#include "objectpool.hpp"
#include "world.hpp"

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/member.hpp>

namespace framework {
class objectmanager final : public input::eventreceiver {
public:
  objectmanager();

  virtual ~objectmanager() = default;

  std::shared_ptr<object> create(const std::string& kind, std::optional<std::reference_wrapper<const std::string>> scope, bool manage = true);

  std::shared_ptr<object> clone(std::shared_ptr<object> matrix);

  void manage(std::shared_ptr<object> object) noexcept;

  void remove(std::shared_ptr<object> object) noexcept;

  std::shared_ptr<object> find(uint64_t id) const noexcept;

  void update(float delta) noexcept;

  void draw() const noexcept;

  void set_resourcemanager(std::shared_ptr<resourcemanager> resourcemanager) noexcept;

  void set_scenemanager(std::shared_ptr<scenemanager> scenemanager) noexcept;

  void set_world(std::shared_ptr<world> world) noexcept;

protected:
  virtual void on_mouse_release(const input::event::mouse::button& event) override;
  virtual void on_mouse_motion(const input::event::mouse::motion& event) override;
  virtual void on_mail(const input::event::mail& event) override;

private:
  struct node {
    uint64_t id;
    std::shared_ptr<object> value;
  };

  struct by_id;
  struct by_seq;
  struct by_ptr;

  using container_t = boost::multi_index::multi_index_container<
    node,
    boost::multi_index::indexed_by<
      boost::multi_index::hashed_unique<
        boost::multi_index::tag<by_id>,
        boost::multi_index::member<node, uint64_t, &node::id>
      >,
      boost::multi_index::sequenced<
        boost::multi_index::tag<by_seq>
      >,
      boost::multi_index::hashed_non_unique<
        boost::multi_index::tag<by_ptr>,
        boost::multi_index::member<node, std::shared_ptr<object>, &node::value>
      >
    >
  >;

  container_t _objects;

  std::shared_ptr<resourcemanager> _resourcemanager;
  std::shared_ptr<scenemanager> _scenemanager;
  std::shared_ptr<world> _world;
  std::unordered_set<uint64_t> _hovering;
  std::atomic<uint64_t> _counter{0};

  std::shared_ptr<uniquepool<envelope, framework::envelope_pool_name>> _envelopepool = envelopepool::instance();
};
}
