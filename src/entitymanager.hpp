#pragma once

#include "common.hpp"
#include "event.hpp"
#include "eventreceiver.hpp"

namespace framework {
class entitymanager : public input::eventreceiver {
public:
  explicit entitymanager(std::shared_ptr<resourcemanager> resourcemanager) noexcept;
  ~entitymanager() = default;

  std::shared_ptr<entity> spawn(const std::string &kind);

  std::shared_ptr<entity> clone(const std::shared_ptr<entity> &matrix) noexcept;

  void destroy(const std::shared_ptr<entity> &entity) noexcept;

  std::shared_ptr<entity> find(uint64_t id) const noexcept;

  void update(float_t delta) noexcept;

  void draw() noexcept;

protected:
  virtual void on_mail(const input::mailevent &event) noexcept override;
  virtual void on_mousebuttondown(const input::mousebuttonevent &event) noexcept override;

private:
  std::shared_ptr<resourcemanager> _resourcemanager;
  std::vector<std::shared_ptr<entity>> _entities;
  std::atomic<uint64_t> _counter{0};
};
}
