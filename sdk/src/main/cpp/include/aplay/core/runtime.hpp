#pragma once

#include <string_view>

namespace aplay::core {

struct RuntimeInfo {
    std::string_view name;
    std::string_view version;
};

RuntimeInfo runtime_info();
std::string_view default_receiver_name();

} // namespace aplay::core
