#include "aplay/sdk.h"

extern "C" const char* aplay_napi_version_stub() {
    return aplay_sdk_version();
}

extern "C" const char* aplay_napi_default_receiver_name_stub() {
    return aplay_sdk_default_receiver_name();
}
