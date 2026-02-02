#pragma once

#include "common.hpp"

#include "noncopyable.hpp"

class eventreceiver;

class eventmanager final : private noncopyable {
public:
  eventmanager();
  virtual ~eventmanager() = default;

  void update(float delta);

  void add_receiver(const std::shared_ptr<eventreceiver>& receiver);

  void remove_receiver(const std::shared_ptr<eventreceiver>& receiver);

  void flush(uint32_t begin_event, uint32_t end_event = 0);

private:
  boost::container::small_vector<std::weak_ptr<eventreceiver>, 16> _receivers;
};
