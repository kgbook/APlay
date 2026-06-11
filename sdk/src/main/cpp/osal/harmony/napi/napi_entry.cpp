#include "sdk.h"

#include <napi/native_api.h>

static napi_value make_string(napi_env env, const char* value) {
    napi_value result = nullptr;
    napi_create_string_utf8(env, value, NAPI_AUTO_LENGTH, &result);
    return result;
}

static napi_value version(napi_env env, napi_callback_info /*info*/) {
    return make_string(env, aplay_sdk_version());
}

static napi_value default_receiver_name(napi_env env, napi_callback_info /*info*/) {
    return make_string(env, aplay_sdk_default_receiver_name());
}

static napi_value init(napi_env env, napi_value exports) {
    napi_property_descriptor descriptors[] = {
        {"version", nullptr, version, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"defaultReceiverName", nullptr, default_receiver_name, nullptr, nullptr, nullptr, napi_default, nullptr},
    };
    napi_define_properties(env, exports, sizeof(descriptors) / sizeof(descriptors[0]), descriptors);
    return exports;
}

static napi_module aplay_napi_module = {
    1,
    0,
    nullptr,
    init,
    "aplay_napi",
    nullptr,
    {nullptr},
};

extern "C" __attribute__((constructor)) void register_aplay_napi_module() {
    napi_module_register(&aplay_napi_module);
}
