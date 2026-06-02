#include "aplay/core/runtime.hpp"

extern "C" const char* aplay_sdk_version() {
    return aplay::core::runtime_info().version.data();
}

extern "C" const char* aplay_sdk_default_receiver_name() {
    return aplay::core::default_receiver_name().data();
}
