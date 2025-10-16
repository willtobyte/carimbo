#pragma once

#include "common.hpp"

#include "event.hpp"
#include "eventreceiver.hpp"
#include "scenemanager.hpp"
#include "resourcemanager.hpp"
#include "objectpool.hpp"
#include "world.hpp"

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
  std::shared_ptr<resourcemanager> _resourcemanager;
  std::shared_ptr<scenemanager> _scenemanager;
  std::shared_ptr<world> _world;
  std::vector<std::shared_ptr<object>> _objects;
  std::unordered_set<uint64_t> _hovering;
  std::atomic<uint64_t> _counter{0};

  std::shared_ptr<uniquepool<envelope, framework::envelope_pool_name>> _envelopepool = envelopepool::instance();
};
}
