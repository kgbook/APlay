include(${CLEAR_VARS})
set(LOCAL_MODULE napi)
set(LOCAL_SRC_FILES
    napi_entry.cpp)
set(LOCAL_INTERFACE_LIBRARIES
    aplay_cpp_sdk
    osal)
set(LOCAL_SHARED_LIBRARIES
    libace_napi.z.so)
set(LOCAL_MODULE_FILENAME
    aplay_napi)
include(${BUILD_SHARED_LIBRARY})
