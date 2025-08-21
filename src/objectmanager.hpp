#pragma once

#include "common.hpp"

#include "event.hpp"
#include "eventreceiver.hpp"
#include "scenemanager.hpp"
#include "resourcemanager.hpp"
#include "objectpool.hpp"

namespace framework {
class objectmanager final : public input::eventreceiver {
public:
  explicit objectmanager(std::shared_ptr<resourcemanager> resourcemanager);
  virtual ~objectmanager() = default;

  std::shared_ptr<object> create(const std::string& kind, std::optional<std::reference_wrapper<const std::string>> scope, bool manage = true);

  std::shared_ptr<object> clone(std::shared_ptr<object> matrix);

  void manage(std::shared_ptr<object> object);

  void unmanage(std::shared_ptr<object> object);

  void destroy(std::shared_ptr<object> object);

  std::shared_ptr<object> find(uint64_t id) const;

  void set_scenemanager(std::shared_ptr<scenemanager> scenemanager);

  void update(float_t delta) noexcept;

  void draw() const noexcept;

protected:
  virtual void on_mouse_press(const input::event::mouse::button& event) override;
  virtual void on_mouse_motion(const input::event::mouse::motion& event) override;
  virtual void on_mail(const input::event::mail& event) override;

private:
  std::shared_ptr<resourcemanager> _resourcemanager;
  std::shared_ptr<scenemanager> _scenemanager;
  std::vector<std::shared_ptr<object>> _objects;
  std::atomic<uint64_t> _counter{0};

  std::shared_ptr<sharedpool<object, framework::object_pool_name>> _objectpool;
  std::shared_ptr<uniquepool<envelope, framework::envelope_pool_name>> _envelopepool;
};
}
