set(APLAY_ANDROID_OSAL_MODULES
    codec
    render
    jni)

foreach(APLAY_ANDROID_OSAL_MODULE IN LISTS APLAY_ANDROID_OSAL_MODULES)
    add_subdirectory(${APLAY_ANDROID_OSAL_MODULE})
endforeach()

include(${CLEAR_VARS})
set(LOCAL_MODULE android)
set(LOCAL_INTERFACE_LIBRARIES
    codec
    render)
include(${BUILD_INTERFACE_LIBRARY})
