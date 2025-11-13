#pragma once

#include "common.hpp"

#include "objectpool.hpp"

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/member.hpp>

namespace framework {
class objectmanager final : public input::eventreceiver, public std::enable_shared_from_this<objectmanager> {
public:
  objectmanager();

  virtual ~objectmanager() = default;

  std::shared_ptr<object> create(std::string_view kind, std::optional<std::string_view> scope, bool manage = true);

  std::shared_ptr<object> clone(std::shared_ptr<object> matrix);

  void manage(std::shared_ptr<object> object);

  bool remove(std::shared_ptr<object> object);

  std::shared_ptr<object> find(uint64_t id) const;

  void update(float delta);

  void draw() const;

  void set_resourcemanager(std::shared_ptr<resourcemanager> resourcemanager);
  void set_scenemanager(std::shared_ptr<scenemanager> scenemanager);
  void set_world(std::shared_ptr<world> world);

protected:
  virtual void on_mouse_release(const input::event::mouse::button& event) override;
  virtual void on_mouse_motion(const input::event::mouse::motion& event) override;
  virtual void on_mail(const input::event::mail& event) override;

private:
  struct by_id;
  struct by_seq;
  struct node {
    std::shared_ptr<framework::object> object;
    explicit node(std::shared_ptr<framework::object> o) : object(std::move(o)) {}
  };

  struct id_key {
    using result_type = uint64_t;
    result_type operator()(const node& n) const {
      return n.object->id();
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

  std::atomic<uint64_t> _counter{0};
  std::shared_ptr<resourcemanager> _resourcemanager;
  std::shared_ptr<scenemanager> _scenemanager;
  std::shared_ptr<world> _world;
  std::unordered_set<uint64_t> _hovering;

  std::shared_ptr<uniquepool<envelope, framework::envelope_pool_name>> _envelopepool = envelopepool::instance();
};
}
