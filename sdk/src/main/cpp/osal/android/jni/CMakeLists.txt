include(${CLEAR_VARS})
set(LOCAL_MODULE aplay_jni)
set(LOCAL_SRC_FILES
    native_entry.cpp)
set(LOCAL_INTERFACE_LIBRARIES
    aplay_cpp_sdk)
set(LOCAL_MODULE_FILENAME
    APlaySdk)
include(${BUILD_SHARED_LIBRARY})
