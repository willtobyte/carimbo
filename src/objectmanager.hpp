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

  bool remove(std::shared_ptr<object> object) noexcept;

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
    std::shared_ptr<framework::object> object;
    explicit node(std::shared_ptr<framework::object> o) : object(std::move(o)) {}
  };

  struct by_id;
  struct by_seq;

  struct id_key {
    using result_type = uint64_t;
    result_type operator()(const node& n) const noexcept {
      return static_cast<uint64_t>(reinterpret_cast<uintptr_t>(n.object.get()));
    }
  };

  using container_t = boost::multi_index::multi_index_container<
    node,
    boost::multi_index::indexed_by<
      boost::multi_index::hashed_unique<
        boost::multi_index::tag<by_id>,
        id_key
      >,
      boost::multi_index::sequenced<
        boost::multi_index::tag<by_seq>
      >
    >
  >;

  container_t _objects;

  std::shared_ptr<resourcemanager> _resourcemanager;
  std::shared_ptr<scenemanager> _scenemanager;
  std::shared_ptr<world> _world;
  std::unordered_set<uint64_t> _hovering;

  std::shared_ptr<uniquepool<envelope, framework::envelope_pool_name>> _envelopepool = envelopepool::instance();
};
}
