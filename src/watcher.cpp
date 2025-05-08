#include "watcher.hpp"

#ifdef SANDBOX
DebounceListener::DebounceListener(rxcpp::subjects::subject<std::monostate>& subject)
  : subject_(subject) {}

void DebounceListener::handleFileAction(
    efsw::WatchID,
    const std::string&,
    const std::string&,
    efsw::Action,
    std::string
) {
  subject_.get_subscriber().on_next(std::monostate{});
}

#endif
