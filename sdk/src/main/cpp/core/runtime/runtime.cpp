#include "sdk.h"

extern "C" const char* aplay_sdk_version() {
    return "0.1.0";
}

extern "C" const char* aplay_sdk_default_receiver_name() {
    return "APlayReceiver";
}
