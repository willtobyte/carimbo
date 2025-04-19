#pragma once

#include "common.hpp"

#include "event.hpp"
#include "eventreceiver.hpp"
#include "object.hpp"
#include "scenemanager.hpp"
#include "resourcemanager.hpp"

namespace framework {
class objectmanager : public input::eventreceiver {
public:
  explicit objectmanager(std::shared_ptr<resourcemanager> resourcemanager) noexcept;
  ~objectmanager() = default;

  std::shared_ptr<object> create(const std::string &scope, const std::string &kind, bool manage = true);

  std::shared_ptr<object> clone(std::shared_ptr<object> matrix) noexcept;

  void manage(std::shared_ptr<object> object) noexcept;

  void unmanage(std::shared_ptr<object> object) noexcept;

  void destroy(std::shared_ptr<object> object) noexcept;

  std::shared_ptr<object> find(uint64_t id) const noexcept;

  void set_scenemanager(std::shared_ptr<scenemanager> scenemanager);

  void update(float_t delta) noexcept;

  void draw() noexcept;

protected:
  virtual void on_mail(const input::event::mail &event) noexcept override;
  virtual void on_mousebuttondown(const input::event::mouse::button &event) noexcept override;
  virtual void on_mousemotion(const input::event::mouse::motion &event) noexcept override;

private:
  std::shared_ptr<resourcemanager> _resourcemanager;
  std::shared_ptr<scenemanager> _scenemanager;
  std::vector<std::shared_ptr<object>> _objects;
  std::atomic<uint64_t> _counter{0};
  bool _dirty{true};
};
}
