set(APLAY_ANDROID_RENDER_MODULES
    arender
    vrender)

foreach(APLAY_ANDROID_RENDER_MODULE IN LISTS APLAY_ANDROID_RENDER_MODULES)
    add_subdirectory(${APLAY_ANDROID_RENDER_MODULE})
endforeach()

include(${CLEAR_VARS})
set(LOCAL_MODULE render)
set(LOCAL_INTERFACE_LIBRARIES
    arender
    vrender)
include(${BUILD_INTERFACE_LIBRARY})
