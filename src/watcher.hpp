#pragma once

#include "common.hpp"

#ifdef SANDBOX
#include <efsw/efsw.hpp>
#include <rxcpp/rx.hpp>

class DebounceListener : public efsw::FileWatchListener {
  public:
    DebounceListener(rxcpp::subjects::subject<std::monostate>& subject);

    void handleFileAction(
        efsw::WatchID,
        const std::string&,
        const std::string&,
        efsw::Action,
        std::string
    ) override;

  private:
    rxcpp::subjects::subject<std::monostate>& subject_;
};

#endif
