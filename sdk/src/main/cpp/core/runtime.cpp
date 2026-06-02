#include "aplay/core/runtime.hpp"

namespace aplay::core {

RuntimeInfo runtime_info() {
    return RuntimeInfo{
        "APlay",
        "0.1.0",
    };
}

std::string_view default_receiver_name() {
    return runtime_info().name;
}

} // namespace aplay::core
